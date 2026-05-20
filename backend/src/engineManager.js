import {spawn} from "bun";
import path from "node:path";
import {fileURLToPath} from "node:url";
import {EventEmitter} from "node:events";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const ENGINE_PATH = path.join(__dirname, "..", "..", "engines", "myengine", "build", "myengine");

export class UciEngine extends EventEmitter {
    constructor(cmd = ENGINE_PATH, options = {}) {
        super();
        this.cmd = cmd;
        this.process = null;
        this.ready = false;
        this.queue = [];
        this.restarts = 0;
        this.maxRestarts = options.maxRestarts ?? 5;
        this.handshakeTimeoutMs = options.handshakeTimeoutMs ?? 2000;
        this.restartDelayMs = options.restartDelayMs ?? 1000;
        this.commandTimeoutBufferMs = options.commandTimeoutBufferMs ?? 2000;
        this.spawnFn = options.spawnFn ?? spawn;
        this.isShuttingDown = false;
    }

    async ensureReady() {
        if (!this.ready) await this.start();
    }

    async start() {
        if (this.process) return;
        this.isShuttingDown = false;

        try {
            console.log(`[Engine] Spawning: ${this.cmd}`);
            this.process = this.spawnFn({
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

            await this._sendCommand("uci", (line) => line === "uciok", null, this.handshakeTimeoutMs);
            await this._sendCommand("isready", (line) => line === "readyok", null, this.handshakeTimeoutMs);

            this.ready = true;
            this.restarts = 0;
            console.log("[Engine] Ready and Listening.");

        } catch (error) {
            console.error("[Engine Critical] Start failed:", error);
            this._handleCrash();
            throw error;
        }
    }

    async stop() {
        this.isShuttingDown = true;
        this.ready = false;

        if (this.process) {
            try {
                await this._sendRaw("quit");
                await new Promise(r => setTimeout(r, 100));
            } catch (_) {
            }

            this.process.kill();
            this.process = null;
        }
        console.log("[Engine] Stopped.");
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
            console.warn(`[Engine] Attempting restart ${this.restarts}/${this.maxRestarts} in ${this.restartDelayMs}ms...`);
            await new Promise(r => setTimeout(r, this.restartDelayMs));
            this.start().catch(e => console.error("Restart failed:", e));
        } else {
            console.error("[Engine] Max restarts exceeded.");
            this.emit("fatal_error", new Error("Max restarts exceeded"));
        }
    }

    async _sendRaw(cmd) {
        if (!this.process) throw new Error("Engine not running");
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

            const timer = setTimeout(() => {
                console.error(`[Engine] Command '${command}' timed out. Engine is hung.`);
                this._handleCrash();
                reject(new Error(`TIMEOUT: ${command}`));
            }, timeoutMs);

            this.queue.push(task);

            this._sendRaw(command).catch(err => {
                clearTimeout(timer);
                reject(err);
            });
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

    async uciNewGame() {
        await this.ensureReady();
        await this._sendRaw("ucinewgame");
        await this._sendCommand("isready", (l) => l === "readyok");
    }

    async position(fen, moves = []) {
        await this.ensureReady();
        let cmd = fen === "startpos" ? "position startpos" : `position fen ${fen}`;
        if (moves.length > 0) cmd += ` moves ${moves.join(" ")}`;
        await this._sendRaw(cmd);
    }

    async go(options = {}) {
        await this.ensureReady();

        const parts = ["go"];
        if (options.depth) parts.push(`depth ${options.depth}`);
        if (options.whiteTime) parts.push(`wtime ${options.whiteTime}`);
        if (options.blackTime) parts.push(`btime ${options.blackTime}`);
        if (options.whiteInc) parts.push(`winc ${options.whiteInc}`);
        if (options.blackInc) parts.push(`binc ${options.blackInc}`);
        if (options.moveTime) parts.push(`movetime ${options.moveTime}`);

        let safeTimeout = options.moveTime ? options.moveTime + this.commandTimeoutBufferMs : (options.whiteTime ? 60000 * 5 : 60000);
        let currentBestMove = "(none)";

        try {
            const response = await this._sendCommand(
                parts.join(" "),
                (line) => line.startsWith("bestmove"),
                (line) => {
                    if (line.startsWith("info") && line.includes(" pv ")) {
                        const moves = line.split(" pv ")[1]?.split(" ");
                        if (moves && moves[0]) currentBestMove = moves[0];
                    }
                },
                safeTimeout,
            );
            return response.split(" ")[1];
        } catch (e) {
            console.error("[Engine] Error during 'go':", e);
            return currentBestMove !== "(none)" ? currentBestMove : "0000";
        }
    }

    async bench(options = {}) {
        await this.ensureReady();

        const parts = ["bench"];
        if (options.depth) parts.push(`depth ${options.depth}`);
        if (options.evalTime) parts.push(`eval ${options.evalTime}`);
        if (options.mode === "time" && options.timeLimit) parts.push(`movetime ${options.timeLimit}`);

        const results = {nps: 0, eps: 0, nodes: 0, time: 0, ordering: 0, qSearch: 0, ttHit: 0, fullOutput: []};
        const predictedTimeout = options.timeLimit ? (options.timeLimit * 2) : 60000;

        await this._sendCommand(
            parts.join(" "),
            (line) => line.includes("Benchmark Complete"),
            (line) => {
                results.fullOutput.push(line);
                const parseVal = (str) => parseFloat(str.split(":")[1]?.trim().replace("%", "") || 0);

                if (line.includes("Global NPS:")) results.nps = parseInt(line.split(":")[1].trim(), 10);
                if (line.includes("EPS:")) results.eps = parseInt(line.split(":")[1].trim(), 10);
                if (line.includes("Total Nodes:")) results.nodes = parseInt(line.split(":")[1].trim(), 10);
                if (line.includes("Move Ordering:")) results.ordering = parseVal(line);
                if (line.includes("Q-Search Load:")) results.qSearch = parseVal(line);
                if (line.includes("TT Hit Rate:")) results.ttHit = parseVal(line);
            },
            predictedTimeout,
        );
        return results;
    }
}

export class EngineManager {
    constructor(options = {}) {
        this.engines = new Map();
        this.engineOptions = options.engineOptions ?? {};
    }

    async registerEngine(id, path) {
        if (this.engines.has(id)) {
            console.warn(`[Manager] Engine ${id} is already registered.`);
            return this.engines.get(id);
        }

        const engine = new UciEngine(path, this.engineOptions);

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
        const engine = this.engines.get(id);
        if (engine) {
            this.engines.delete(id);
            await engine.stop().catch(e => console.error(`[Manager] Error stopping engine ${id}:`, e));
        }
    }

    async shutdownAll() {
        console.log(`[Manager] Shutting down all ${this.engines.size} engines...`);

        const stopPromises = Array.from(this.engines.values()).map(engine =>
            engine.stop().catch(e => console.error("[Manager] Error during mass shutdown:", e))
        );

        this.engines.clear();

        await Promise.allSettled(stopPromises);
        console.log("[Manager] All engines shut down successfully.");
    }
}

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