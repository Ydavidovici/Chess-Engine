import { spawn } from "bun";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const TOOLS_DIR = path.join(__dirname, "../../../tools");
const ENGINES_DIR = path.join(__dirname, "../../../engines");

const CUTECHESS = path.join(TOOLS_DIR, "cutechess-1.4.0-win64/cutechess-cli.exe");
const BOOK_PATH = path.join(TOOLS_DIR, "UHO_4060_v1.epd");
const MY_ENGINE = path.join(ENGINES_DIR, "myengine/build/myengine.exe");
const TSCP_PATH = path.join(ENGINES_DIR, "tscp/tscp181/tscp181.exe");
const STOCKFISH_PATH = path.join(ENGINES_DIR, "stockfish/stockfish/stockfish-windows-x86-64-avx2.exe");

// =========================================================

async function runTournament() {
    console.log("🔥 Starting 1,000-Game Baseline Gauntlet! 🔥\n");

    const args = [
        // Tells Cutechess to make Engine #1 play against everyone else, but they don't play each other
        "-tournament", "gauntlet",

        // Add proto=uci to MyEngine and Stockfish
        "-engine", `name=MyEngine`, `cmd=${MY_ENGINE}`, `proto=uci`,
        "-engine", `name=Stockfish_300`, `cmd=${STOCKFISH_PATH}`, `option.Skill Level=0`, `nodes=10`, `proto=uci`,
        "-engine", `name=Stockfish_800`, `cmd=${STOCKFISH_PATH}`, `option.Skill Level=0`, `nodes=250`, `proto=uci`,
        "-engine", `name=Stockfish_1200`, `cmd=${STOCKFISH_PATH}`, `option.Skill Level=0`, `nodes=2000`, `proto=uci`,

        // Add proto=xboard to TSCP
        "-engine", `name=TSCP_1700`, `cmd=${TSCP_PATH}`, `proto=xboard`,

        "-each",
        // "proto=uci", <--- REMOVE THIS LINE
        "tc=10+0.1",

        "-rounds", "125",
        "-games", "2",
        "-repeat",
        "-concurrency", "4",
        "-ratinginterval", "10",
        "-pgnout", "baseline_1000_games.pgn",

        "-openings",
        `file=${BOOK_PATH}`,
        "format=epd",
        "order=random",
        "plies=16"
    ];

    const cutechessProcess = spawn({
        cmd: [CUTECHESS, ...args],
        stdout: "pipe",
        stderr: "inherit",
    });

    const decoder = new TextDecoder();
    for await (const chunk of cutechessProcess.stdout) {
        process.stdout.write(decoder.decode(chunk));
    }

    const exitCode = await cutechessProcess.exited;
    console.log(`\nTournament finished with code ${exitCode}.`);
}

runTournament().catch(console.error);