import { spawn } from "bun";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);


const ENGINE_PATH = path.join(__dirname, "..", "..", "engines", "myengine", "build", "myengine");

export class UciEngine {
    constructor(cmd = ENGINE_PATH) {
        this.cmd = cmd;
        this.proc = null;
        this.ready = false;
        this.queue = [];
        this.current = null;
    }

    async start() {
        if (this.proc) return;

        this.proc = spawn({
            cmd: [this.cmd],
            stdin: "pipe",
            stdout: "pipe",
            stderr: "inherit",
        });

        this._readLoop();

        await this._sendCommand("uci", (line) => line.startsWith("uciok"));
        await this._sendCommand("isready", (line) => line.startsWith("readyok"));

        this.ready = true;
        console.log("[engine] UCI engine ready");
    }

    async stop() {
        if (!this.proc) return;
        try {
            await this._sendRaw("quit");
        } catch (_) {}
        this.proc.kill();
        this.proc = null;
        this.ready = false;
    }

    async bestMoveFromFen(fen, { depth = 15 } = {}) {
        if (!this.ready) {
            await this.start();
        }

        await this._sendCommand(`position fen ${fen}`, () => false);
        const bestmoveLine = await this._sendCommand(
            `go depth ${depth}`,
            (line) => line.startsWith("bestmove")
        );

        const parts = bestmoveLine.split(/\s+/);
        const move = parts[1] || null;
        return move;
    }


    async _sendRaw(cmd) {
        if (!this.proc) throw new Error("engine not started");
        this.proc.stdin.write(cmd + "\n");
    }

    async _sendCommand(cmd, donePredicate) {
        return new Promise(async (resolve, reject) => {
            const item = { donePredicate, resolve, reject };
            this.queue.push(item);
            try {
                await this._sendRaw(cmd);
            } catch (e) {
                reject(e);
            }
        });
    }

    async _readLoop() {
        if (!this.proc) return;

        const decoder = new TextDecoder();
        let buffer = "";

        for await (const chunk of this.proc.stdout) {
            buffer += decoder.decode(chunk);

            let idx;
            while ((idx = buffer.indexOf("\n")) >= 0) {
                const line = buffer.slice(0, idx).trim();
                buffer = buffer.slice(idx + 1);

                if (!line) continue;

                if (this.queue.length > 0) {
                    const current = this.queue[0];
                    if (current.donePredicate(line)) {
                        this.queue.shift().resolve(line);
                    }
                }
            }
        }
    }
}
