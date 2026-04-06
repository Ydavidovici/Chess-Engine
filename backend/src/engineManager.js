import { spawn } from "bun";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { EventEmitter } from "node:events";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const ENGINE_PATH = path.join(__dirname, "..", "..", "engines", "myengine", "build", "myengine");

// ============================================================================
// 1. INDIVIDUAL ENGINE WRAPPER
// ============================================================================
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

            if (!this.process.pid) throw new Error("Failed to spawn engine process.");

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
            console.error("[Engine] Max restarts exceeded.");
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
        } catch (_) {}

        if (this.process) this.process.kill();
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
        let cmd = fen === "startpos" ? "position startpos" : `position fen ${fen}`;
        if (moves.length > 0) cmd += ` moves ${moves.join(" ")}`;
        await this._sendRaw(cmd);
    }

    async go(options = {}) {
        if (!this.ready) await this.start();

        let cmd = "go";
        if (options.depth) cmd += ` depth ${options.depth}`;
        if (options.whiteTime) cmd += ` wtime ${options.whiteTime}`;
        if (options.blackTime) cmd += ` btime ${options.blackTime}`;
        if (options.whiteInc) cmd += ` winc ${options.whiteInc}`;
        if (options.blackInc) cmd += ` binc ${options.blackInc}`;
        if (options.moveTime) cmd += ` movetime ${options.moveTime}`;

        let safeTimeout = options.moveTime ? options.moveTime + 2000 : (options.whiteTime ? 60000 * 5 : 60000);
        let currentBestMove = "(none)";

        try {
            const response = await this._sendCommand(
                cmd,
                (line) => line.startsWith("bestmove"),
                (line) => {
                    if (line.startsWith("info") && line.includes(" pv ")) {
                        const moves = line.split(" pv ")[1]?.split(" ");
                        if (moves && moves[0]) currentBestMove = moves[0];
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

    async _sendRaw(cmd) {
        if (!this.process) throw new Error("Engine not started");
        try {
            this.process.stdin.write(cmd + "\n");
            this.process.stdin.flush();
        } catch (err) {
            this._handleCrash();
            throw err;
        }
    }

    _sendCommand(command, stopCondition, callback, timeoutMs = 2000) {
        return new Promise((resolve, reject) => {
            if (!this.process) return reject(new Error("Engine not running"));

            const task = {
                command,
                donePredicate: stopCondition,
                callback: callback,
                resolve: (val) => { clearTimeout(timer); resolve(val); },
                reject: (err) => { clearTimeout(timer); reject(err); },
            };

            const timer = setTimeout(() => {
                const index = this.queue.indexOf(task);
                if (index !== -1) {
                    this.queue.splice(index, 1);
                    console.error(`[Engine] Command '${command}' timed out.`);
                    resolve("TIMEOUT");
                }
            }, timeoutMs);

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
                    const line = buffer.slice(0, idx).trim();
                    buffer = buffer.slice(idx + 1);

                    if (!line || this.queue.length === 0) continue;

                    const currentTask = this.queue[0];
                    if (currentTask.callback) currentTask.callback(line);

                    if (currentTask.donePredicate(line)) {
                        this.queue.shift();
                        currentTask.resolve(line);
                    }
                }
            }
        } catch (err) {
            console.error("[Engine] stdout stream error:", err);
            throw err;
        } finally {
            this._handleCrash();
        }
    }

    async bench(options = {}) {
        if (!this.ready) await this.start();
        let command = "bench";
        if (options.depth) command += ` depth ${options.depth}`;
        if (options.evalTime) command += ` eval ${options.evalTime}`;
        if (options.mode === "time" && options.timeLimit) command += ` movetime ${options.timeLimit}`;

        const results = { nps: 0, eps: 0, nodes: 0, time: 0, ordering: 0, qSearch: 0, ttHit: 0, fullOutput: [] };
        const predictedTimeout = options.timeLimit ? (options.timeLimit * 2) : 60000;

        await this._sendCommand(
            command,
            (line) => line.includes("Benchmark Complete"),
            (line) => {
                results.fullOutput.push(line);
                const parseVal = (str) => parseFloat(str.split(":")[1]?.trim().replace("%", "") || 0);

                if (line.includes("Global NPS:")) results.nps = parseInt(line.split(":")[1].trim());
                if (line.includes("EPS:")) results.eps = parseInt(line.split(":")[1].trim());
                if (line.includes("Total Nodes:")) results.nodes = parseInt(line.split(":")[1].trim());
                if (line.includes("Move Ordering:")) results.ordering = parseVal(line);
                if (line.includes("Q-Search Load:")) results.qSearch = parseVal(line);
                if (line.includes("TT Hit Rate:")) results.ttHit = parseVal(line);
            },
            predictedTimeout,
        );
        return results;
    }
}

// ============================================================================
// 2. ENGINE REGISTRY MANAGER
// ============================================================================
export class EngineManager {
    constructor() {
        this.engines = new Map();
    }

    async registerEngine(id, path) {
        if (this.engines.has(id)) {
            console.warn(`[Manager] Engine ${id} is already registered.`);
            return this.engines.get(id);
        }

        const engine = new UciEngine(path);

        engine.on("fatal_error", (err) => {
            console.error(`[Manager] Engine ${id} died permanently:`, err);
            this.engines.delete(id);
        });

        await engine.start();
        this.engines.set(id, engine);
        console.log(`[Manager] Successfully registered engine: ${id}`);
        return engine;
    }

    getEngine(id) {
        if (!this.engines.has(id)) throw new Error(`Engine ${id} not found.`);
        return this.engines.get(id);
    }

    async shutdownEngine(id) {
        if (this.engines.has(id)) {
            await this.engines.get(id).stop();
            this.engines.delete(id);
        }
    }

    async shutdownAll() {
        console.log(`[Manager] Shutting down all ${this.engines.size} engines...`);
        for (const engine of this.engines.values()) {
            await engine.stop();
        }
        this.engines.clear();
    }
}

// ============================================================================
// 3. TOURNAMENT RUNNER (CUTECHESS)
// ============================================================================
export class CutechessManager extends EventEmitter {
    constructor(cutechessPath) {
        super();
        this.cmd = cutechessPath;
        this.process = null;
    }

    async runGauntlet({myEngine, opponents, timeControl = "10+0.1", rounds = 50, concurrency = 4, pgnOut = "gauntlet.pgn", openingBook = null}) {
        if (this.process) throw new Error("A tournament is already running!");

        console.log(`[Cutechess] Starting Gauntlet: ${myEngine.name} vs ${opponents.length} opponents.`);

        const args = ["-engine", `name=${myEngine.name}`, `cmd=${myEngine.path}`];
        for (const opp of opponents) {
            args.push("-engine", `name=${opp.name}`, `cmd=${opp.path}`);
            // Check if we passed specific node/skill restrictions for this opponent
            if (opp.args) args.push(...opp.args);
        }

        args.push(
            "-each", `tc=${timeControl}`,
            "-rounds", rounds.toString(),
            "-games", "2",
            "-repeat",
            "-concurrency", concurrency.toString(),
            "-ratinginterval", "10",
            "-pgnout", pgnOut,
        );

        if (openingBook) {
            args.push("-openings", `file=${openingBook.file}`, `format=${openingBook.format}`, "order=random", "plies=16");
        }

        return new Promise((resolve, reject) => {
            try {
                this.process = spawn({
                    cmd: [this.cmd, ...args],
                    stdout: "pipe",
                    stderr: "pipe",
                });

                const decoder = new TextDecoder();

                (async () => {
                    for await (const chunk of this.process.stdout) {
                        const text = decoder.decode(chunk);
                        process.stdout.write(text);
                        if (text.includes("Elo difference:")) this.emit("elo_update", text);
                    }
                })();

                (async () => {
                    for await (const chunk of this.process.stderr) {
                        console.error(`[Cutechess Error] ${decoder.decode(chunk)}`);
                    }
                })();

                this.process.exited.then((code) => {
                    console.log(`[Cutechess] Tournament finished with exit code ${code}.`);
                    this.process = null;
                    if (code === 0) resolve(pgnOut);
                    else reject(new Error(`Cutechess exited with code ${code}`));
                });

            } catch (err) {
                console.error("[Cutechess] Failed to spawn:", err);
                reject(err);
            }
        });
    }

    stop() {
        if (this.process) {
            console.log("[Cutechess] Aborting tournament...");
            this.process.kill();
            this.process = null;
        }
    }
}