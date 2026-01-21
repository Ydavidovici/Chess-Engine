import {spawn} from "bun";
import path from "node:path";
import {fileURLToPath} from "node:url";
import {EventEmitter} from "node:events";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const ENGINE_PATH = path.join(__dirname, "..", "..", "engines", "myengine", "build", "myengine");

/**
 * Robust UCI Engine Wrapper
 * Features: Auto-restart on crash, command queueing, strict timeout management.
 */
export class UciEngine extends EventEmitter {
    constructor(cmd = ENGINE_PATH) {
        super();
        this.cmd = cmd;
        this.process = null;
        this.ready = false;
        this.queue = [];
        this.restarts = 0;
        this.maxRestarts = 5;
        this.isShuttingDown = false;
    }

    /**
     * Starts the engine process and initializes UCI mode.
     */
    async start() {
        if (this.process) return;
        this.isShuttingDown = false;

        try {
            console.log(`[Engine] Spawning: ${this.cmd}`);
            this.process = spawn({
                cmd: [this.cmd],
                stdin: "pipe",
                stdout: "pipe",
                stderr: "inherit",
            });

            if (!this.process.pid) {
                throw new Error("Failed to spawn engine process (PID is null).");
            }

            this._readLoop().catch((err) => {
                console.error("[Engine Error] readLoop crashed:", err);
                this._handleCrash();
            });

            await this._sendCommand("uci", (line) => line === "uciok", null, 2000);
            await this._sendCommand("isready", (line) => line === "readyok", null, 2000);

            this.ready = true;
            this.restarts = 0;
            console.log("[Engine] Ready and Listening.");

        } catch (error) {
            console.error("[Engine Critical] Start failed:", error);
            this._handleCrash();
            throw error;
        }
    }

    /**
     * Internal: Handles engine crashes by attempting to restart.
     */
    async _handleCrash() {
        if (this.isShuttingDown) return;

        this.ready = false;
        this.process = null;

        if (this.queue.length > 0) {
            console.warn(`[Engine] Clearing ${this.queue.length} pending commands due to crash.`);
            this.queue.forEach(task => task.reject(new Error("Engine crashed")));
            this.queue = [];
        }

        if (this.restarts < this.maxRestarts) {
            this.restarts++;
            console.warn(`[Engine] Attempting restart ${this.restarts}/${this.maxRestarts} in 1s...`);
            await new Promise(r => setTimeout(r, 1000));
            this.start().catch(e => console.error("Restart failed:", e));
        } else {
            console.error("[Engine] Max restarts exceeded. Manual intervention required.");
            this.emit("fatal_error", new Error("Max restarts exceeded"));
        }
    }

    async stop() {
        this.isShuttingDown = true;
        if (!this.process) return;

        this.ready = false;

        try {
            this.process.stdin.write("quit\n");
            await new Promise(r => setTimeout(r, 100));
        } catch (_) {
        }

        if (this.process) {
            this.process.kill();
        }
        this.process = null;
        console.log("[Engine] Stopped.");
    }

    async uciNewGame() {
        if (!this.ready) await this.start();
        await this._sendRaw("ucinewgame");
        await this._sendCommand("isready", (l) => l === "readyok");
    }

    async position(fen, moves = []) {
        if (!this.ready) await this.start();

        let cmd;
        if (fen === "startpos") {
            cmd = "position startpos";
        } else {
            cmd = `position fen ${fen}`;
        }

        if (moves.length > 0) {
            cmd += ` moves ${moves.join(" ")}`;
        }

        await this._sendRaw(cmd);
    }

    /**
     * Calculates the best move with safety fallbacks.
     */
    async go(options = {}) {
        if (!this.ready) await this.start();

        let cmd = "go";
        if (options.depth) cmd += ` depth ${options.depth}`;
        if (options.whiteTime) cmd += ` wtime ${options.whiteTime}`;
        if (options.blackTime) cmd += ` btime ${options.blackTime}`;
        if (options.whiteInc) cmd += ` winc ${options.whiteInc}`;
        if (options.blackInc) cmd += ` binc ${options.blackInc}`;
        if (options.moveTime) cmd += ` movetime ${options.moveTime}`;

        let safeTimeout = 60000;
        if (options.moveTime) {
            safeTimeout = options.moveTime + 2000;
        } else if (options.whiteTime || options.blackTime) {
            safeTimeout = 60000 * 5;
        }

        let currentBestMove = "(none)";

        try {
            const response = await this._sendCommand(
                cmd,
                (line) => line.startsWith("bestmove"),
                (line) => {
                    if (line.startsWith("info") && line.includes(" pv ")) {
                        const parts = line.split(" pv ");
                        if (parts[1]) {
                            const moves = parts[1].split(" ");
                            if (moves[0]) currentBestMove = moves[0];
                        }
                    }
                },
                safeTimeout,
            );

            if (response === "TIMEOUT") {
                console.warn(`[Engine] Search timed out. Fallback to pv: ${currentBestMove}`);
                await this._sendRaw("stop");
                return currentBestMove;
            }

            return response.split(" ")[1];
        } catch (e) {
            console.error("[Engine] Error during 'go':", e);
            return currentBestMove !== "(none)" ? currentBestMove : "0000";
        }
    }

    /**
     * Sends command to stdin.
     */
    async _sendRaw(cmd) {
        if (!this.process) throw new Error("Engine not started");
        try {
            this.process.stdin.write(cmd + "\n");
            this.process.stdin.flush();
        } catch (err) {
            console.error(`[Engine] Failed to write '${cmd}':`, err);
            this._handleCrash();
            throw err;
        }
    }

    /**
     * Core Command Handler with Promise Queue
     */
    _sendCommand(command, stopCondition, callback, timeoutMs = 2000) {
        return new Promise((resolve, reject) => {
            if (!this.process) return reject(new Error("Engine not running"));

            // Safety timer
            const timer = setTimeout(() => {
                const index = this.queue.indexOf(task);
                if (index !== -1) {
                    this.queue.splice(index, 1);
                    console.error(`[Engine] Command '${command}' timed out (${timeoutMs}ms).`);
                    resolve("TIMEOUT");
                }
            }, timeoutMs);

            const task = {
                command,
                donePredicate: stopCondition,
                callback: callback,
                resolve: (val) => {
                    clearTimeout(timer);
                    resolve(val);
                },
                reject: (err) => {
                    clearTimeout(timer);
                    reject(err);
                },
            };

            this.queue.push(task);

            try {
                this.process.stdin.write(command + "\n");
                this.process.stdin.flush();
            } catch (err) {
                clearTimeout(timer);
                const idx = this.queue.indexOf(task);
                if (idx !== -1) this.queue.splice(idx, 1);
                reject(err);
            }
        });
    }

    async _readLoop() {
        if (!this.process) return;

        const decoder = new TextDecoder();
        let buffer = "";

        try {
            for await (const chunk of this.process.stdout) {
                buffer += decoder.decode(chunk);

                let idx;
                while ((idx = buffer.indexOf("\n")) >= 0) {
                    const raw = buffer.slice(0, idx);
                    const line = raw.trim();
                    buffer = buffer.slice(idx + 1);

                    if (!line) continue;

                    if (this.queue.length > 0) {
                        const currentTask = this.queue[0];

                        if (currentTask.callback) {
                            try {
                                currentTask.callback(line);
                            } catch (e) {
                                console.error("[Engine] Callback error:", e);
                            }
                        }

                        let done = false;
                        try {
                            done = currentTask.donePredicate(line);
                        } catch (e) {
                            console.error("[Engine] Predicate error:", e);
                        }

                        if (done) {
                            this.queue.shift();
                            currentTask.resolve(line);
                        }
                    }
                }
            }
        } catch (err) {
            console.error("[Engine] stdout stream error:", err);
            throw err;
        } finally {
            console.log("[Engine] stdout stream closed.");
            this._handleCrash();
        }
    }
}