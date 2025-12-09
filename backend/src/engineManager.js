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
        } catch (_) {
        }
        this.process.kill();
        this.process = null;
        this.ready = false;
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
        const bestmoveLine = await this._sendCommand(
            `bestmovefromfen ${fen}`,
            (line) => {
                return line.startsWith("bestmove");
            },
        );
        console.log("[engine wrapper] resolved bestmove line:", bestmoveLine);
        const move = bestmoveLine.slice("bestmove".length).trim();
        return move;
    }


}
