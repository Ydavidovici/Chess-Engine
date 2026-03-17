import {spawn} from "bun";
import path from "node:path";
import {fileURLToPath} from "node:url";
import {execSync} from "node:child_process";
import fs from "node:fs";


const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const TOOLS_DIR = path.join(__dirname, "../../../tools");
const ENGINES_DIR = path.join(__dirname, "../../../engines");

const CUTECHESS = path.join(TOOLS_DIR, "cutechess-1.4.0-win64/cutechess-cli.exe");
const BOOK_PATH = path.join(TOOLS_DIR, "UHO_4060_v1.epd");
const MY_ENGINE = path.join(ENGINES_DIR, "myengine/build/myengine.exe");
const TSCP_PATH = path.join(ENGINES_DIR, "tscp/tscp181/tscp181.exe");
const STOCKFISH_PATH = path.join(ENGINES_DIR, "stockfish/stockfish/stockfish-windows-x86-64-avx2.exe");
const STORAGE_DIR = path.join(__dirname, "../storage");

function cleanupEngines() {
    console.log("\n🧹 Sweeping up orphaned engine processes...");
    try {
        execSync("taskkill /F /IM myengine.exe /T", { stdio: "ignore" });
        execSync("taskkill /F /IM cutechess-cli.exe /T", { stdio: "ignore" });
        execSync("taskkill /F /IM tscp181.exe /T", { stdio: "ignore" });
        console.log("✅ Cleanup complete.");
    } catch (e) {}
}

process.on("SIGINT", () => { cleanupEngines(); process.exit(0); });
process.on("exit", () => { cleanupEngines(); });

// =========================================================

async function runTournament() {
    console.log("🔥 Starting 1,000-Game Baseline Gauntlet! 🔥\n");

    if (!fs.existsSync(STORAGE_DIR)) {
        fs.mkdirSync(STORAGE_DIR, { recursive: true });
        console.log(`📁 Created new storage directory at: ${STORAGE_DIR}`);
    }

    const now = new Date();
    const pad = (n) => n.toString().padStart(2, '0');
    const timestamp = `${now.getFullYear()}-${pad(now.getMonth() + 1)}-${pad(now.getDate())}_${pad(now.getHours())}-${pad(now.getMinutes())}-${pad(now.getSeconds())}`;
    const pgnFilename = path.join(STORAGE_DIR, `tournament_${timestamp}.pgn`);

    const args = [
        "-tournament", "gauntlet",

        "-engine", `name=MyEngine`, `cmd=${MY_ENGINE}`, `proto=uci`,
        "-engine", `name=Stockfish_300`, `cmd=${STOCKFISH_PATH}`, `option.Skill Level=0`, `nodes=10`, `proto=uci`,
        "-engine", `name=Stockfish_800`, `cmd=${STOCKFISH_PATH}`, `option.Skill Level=0`, `nodes=250`, `proto=uci`,
        "-engine", `name=Stockfish_1200`, `cmd=${STOCKFISH_PATH}`, `option.Skill Level=0`, `nodes=2000`, `proto=uci`,

        "-each",
        "tc=10+0.1",

        "-rounds", "125",
        "-games", "2",
        "-repeat",
        "-concurrency", "4",
        "-ratinginterval", "10",
        "-pgnout", pgnFilename,

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