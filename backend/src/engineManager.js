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
            cmd: [this.cmd], stdin: "pipe", stdout: "pipe", stderr: "inherit",
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
        } catch (_) {
        }
        this.process.kill();
        this.process = null;
        this.ready = false;
    }

    async uciNewGame() {
        await this._sendRaw("ucinewgame");
        await this._sendCommand("isready", (l) => l === "readyok");
    }

    /**
     * @param {string} fen "startpos" or a FEN string
     * @param {string[]} moves Array of move strings ["e2e4", "e7e5"]
     */
    async position(fen, moves = []) {
        let cmd = `position ${fen}`;
        if (moves.length > 0) {
            cmd += ` moves ${moves.join(" ")}`;
        }
        await this._sendRaw(cmd);
    }

    /**
     * @param {Object} options { depth, wtime, btime, movetime }
     */
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


    async _sendCommand(cmd, donePredicate) {
        return new Promise(async (resolve, reject) => {
            const item = {donePredicate, resolve, reject};
            this.queue.push(item);
            try {
                await this._sendRaw(cmd);
            } catch (e) {
                reject(e);
            }
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
                    console.log("[engine queue] size:", this.queue.length);
                    const current = this.queue[0];
                    let done = false;
                    try {
                        done = current.donePredicate(line);
                    } catch (e) {
                        console.error("[engine queue] predicate error:", e);
                    }
                    if (done) {
                        const item = this.queue.shift();
                        item.resolve(line);
                    }
                } else {
                    console.log("[engine queue] empty, ignoring line");
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
}
