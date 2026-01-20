import {spawn} from "bun";
import path from "node:path";
import {fileURLToPath} from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const ENGINE_PATH = path.join(__dirname, "..", "..", "engines", "myengine", "build", "myengine");

export class UciEngine {
    constructor(cmd = ENGINE_PATH) {
        this.cmd = cmd;
        this.process = null;
        this.ready = false;
        this.queue = [];
    }

    async start() {
        if (this.process) return;

        this.process = spawn({
            cmd: [this.cmd],
            stdin: "pipe",
            stdout: "pipe",
            stderr: "inherit",
        });

        this._readLoop().catch((err) => {
            console.error("[engine] readLoop error:", err);
        });

        console.log("[engine >>] uci");
        await this._sendCommand("uci", (line) => line.startsWith("uciok"));

        console.log("[engine >>] isready");
        await this._sendCommand("isready", (line) => line.startsWith("readyok"));

        this.ready = true;
        console.log("[engine] start complete, ready =", this.ready);
    }


    async stop() {
        if (!this.process) return;
        try {
            await this._sendRaw("quit");
        } catch (_) {}
        this.process.kill();
        this.process = null;
        this.ready = false;
    }

    async uciNewGame() {
        await this._sendRaw("ucinewgame");
        await this._sendCommand("isready", (l) => l === "readyok");
    }

    async position(fen, moves = []) {
        let cmd = `position ${fen}`;
        if (moves.length > 0) {
            cmd += ` moves ${moves.join(" ")}`;
        }
        await this._sendRaw(cmd);
    }

    async go(options = {}) {
        let cmd = "go";
        if (options.depth) cmd += ` depth ${options.depth}`;
        if (options.wtime) cmd += ` wtime ${options.wtime}`;
        if (options.btime) cmd += ` btime ${options.btime}`;
        if (options.movetime) cmd += ` movetime ${options.movetime}`;

        const line = await this._sendCommand(cmd, (l) => l.startsWith("bestmove"));
        return line.split(" ")[1];
    }

    async _sendRaw(cmd) {
        if (!this.process) throw new Error("engine not started");
        this.process.stdin.write(cmd + "\n");
    }

    /**
     * Sends a command and waits for a specific response line.
     * @param {string} command - The UCI command to send
     * @param {function} stopCondition - Function(line) => bool. Returns true when command is done.
     * @param {function} callback - Function(line). Called for every line of output.
     * @param {number} timeoutMs - (Optional) How long to wait before giving up. Default 1000ms.
     */
    _sendCommand(command, stopCondition, callback, timeoutMs = 1000) {
        return new Promise((resolve, reject) => {
            if (!this.process) return reject(new Error("Engine not running"));

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
                }
            };

            this.queue.push(task);
            this.process.stdin.write(command + "\n");

            const timer = setTimeout(() => {
                const index = this.queue.indexOf(task);
                if (index !== -1) {
                    this.queue.splice(index, 1);
                    console.warn(`[Engine] Command '${command}' timed out after ${timeoutMs}ms. Returning partial data.`);
                    resolve("TIMEOUT");
                }
            }, timeoutMs);
        });
    }

    async _readLoop() {
        if (!this.process) return;

        const decoder = new TextDecoder();
        let buffer = "";

        for await (const chunk of this.process.stdout) {
            buffer += decoder.decode(chunk);

            let idx;
            while ((idx = buffer.indexOf("\n")) >= 0) {
                const raw = buffer.slice(0, idx);
                const line = raw.trim();
                buffer = buffer.slice(idx + 1);

                if (!line) continue;

                console.log("[engine <<]", line);

                if (this.queue.length > 0) {
                    const currentTask = this.queue[0];

                    if (currentTask.callback) {
                        try {
                            currentTask.callback(line);
                        } catch (e) {
                            console.error("Callback error:", e);
                        }
                    }

                    let done = false;
                    try {
                        done = currentTask.donePredicate(line);
                    } catch (e) {
                        console.error("[engine queue] predicate error:", e);
                    }

                    if (done) {
                        this.queue.shift();
                        currentTask.resolve(line);
                    }
                }
            }
        }
    }

    async bestMoveFromFen(fen) {
        if (!this.ready) {
            console.log("[engine wrapper] start() from bestMoveFromFen");
            await this.start();
        }
        console.log("[engine wrapper] queue size before sending:", this.queue.length);
        const bestmoveLine = await this._sendCommand(`bestmovefromfen ${fen}`, (line) => {
            return line.startsWith("bestmove");
        });
        console.log("[engine wrapper] resolved bestmove line:", bestmoveLine);
        const move = bestmoveLine.slice("bestmove".length).trim();
        return move;
    }

    async printBoard(fen) {
        console.log("fen", fen);
        return await this._sendCommand(`printboard ${fen}`, (line) => {
            return line.startsWith("printboard_done");
        });
    }

    async makeMove(fen, move) {
        console.log("making move", move);
        return await this._sendCommand(`makemove ${fen} ${move}`, (line) => {
            return line.startsWith("move_made");
        });
    }

    static async playMatch(white, black, options) {
        console.log(`\n=== MATCH START: ${white.name} (White) vs ${black.name} (Black) ===`);

        await white.start();
        await black.start();

        await white.uciNewGame();
        await black.uciNewGame();

        const moves = [];
        let turn = "white";
        const maxMoves = options.maxMoves || 200;

        try {
            while (moves.length < maxMoves) {
                const activeEngine = turn === "white" ? white : black;

                await activeEngine.position("startpos", moves);

                const bestMove = await activeEngine.go({depth: options.depth});

                if (!bestMove || bestMove === "(none)" || bestMove === "0000") {
                    console.log(`Game Over. ${activeEngine.name} cannot move.`);
                    break;
                }

                console.log(`${moves.length + 1}. ${turn === "white" ? "White" : "Black"} (${activeEngine.name}): ${bestMove}`);
                moves.push(bestMove);

                turn = turn === "white" ? "black" : "white";
            }
        } catch (error) {
            console.error("Match aborted due to error:", error);
        } finally {
            console.log(`=== MATCH END ===`);
            console.log(`Moves: ${moves.join(" ")}`);
            await white.stop();
            await black.stop();
        }
    }

    static async playLichessGame(engine, gameId, apiToken) {
        console.log(`[Lichess Stub] Connecting to game ${gameId}...`);
        await engine.start();
        console.log("[Lichess Stub] Connected. Waiting for events...");
    }

    /**
     * Runs the benchmark suite with specific settings.
     * @param {Object} options Configuration for the bench
     * @param {number} options.depth - Search depth (default: 9)
     * @param {number} options.evalTime - Time in ms for eval test (default: 2000)
     * @param {string} options.mode - "depth" or "time"
     * @param {number} options.timeLimit - Time in ms (if mode is "time")
     */
    async bench(options = {}) {
        if (!this.ready) await this.start();

        let command = "bench";
        if (options.depth) command += ` depth ${options.depth}`;
        if (options.evalTime) command += ` eval ${options.evalTime}`;
        if (options.mode === 'time' && options.timeLimit) {
            command += ` movetime ${options.timeLimit}`;
        }

        let expectedDuration = (options.evalTime || 2000);

        if (options.mode === 'time' && options.timeLimit) {
            expectedDuration += (options.timeLimit * 3);
        } else {
            expectedDuration += 60000;
        }

        if (options.evalTime) command += ` eval ${options.evalTime}`;

        let calculatedTimeout = expectedDuration * 1.5 + 20000;

        const totalTimeout = Math.max(calculatedTimeout, 60000);

        console.log(`[Engine] Bench Configured. Timeout set to: ${totalTimeout}ms (${(totalTimeout/1000).toFixed(0)}s)`);

        const results = {
            nps: 0, eps: 0, nodes: 0, time: 0, ordering: 0, qSearch: 0, ttHit: 0,
            fullOutput: [],
            isPartial: false
        };

        const status = await this._sendCommand(command,
            (line) => line.includes("Benchmark Complete"),
            (line) => {
                results.fullOutput.push(line);

                const parseVal = (str) => {
                    const parts = str.split(":")[1];
                    return parts ? parseFloat(parts.trim().replace('%', '')) : 0;
                };

                if (line.includes("Global NPS:")) results.nps = parseInt(line.split(":")[1].trim());
                if (line.includes("EPS:")) results.eps = parseInt(line.split(":")[1].trim());
                if (line.includes("Total Nodes:")) results.nodes = parseInt(line.split(":")[1].trim());
                if (line.includes("Total Time:")) results.time = parseFloat(line.split(":")[1].trim().replace('s',''));
                if (line.includes("Move Ordering:")) results.ordering = parseVal(line);
                if (line.includes("Q-Search Load:")) results.qSearch = parseVal(line);
                if (line.includes("TT Hit Rate:")) results.ttHit = parseVal(line);
            },
            totalTimeout
        );

        if (status === "TIMEOUT") {
            results.isPartial = true;
            console.warn("[Engine] Benchmark timed out. Returning gathered results.");
        }

        return results;
    }

    async cancel() {
        console.log("[Engine] Cancel requested. resetting...");

        if (this.queue.length > 0) {
            this.queue.forEach(task => task.reject(new Error("Cancelled by user")));
            this.queue = [];
        }

        await this.stop();
        console.log("[Engine] Process killed. Ready for fresh start.");
    }
}
