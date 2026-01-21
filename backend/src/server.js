import express from "express";
import cors from "cors";
import path from "node:path";
import {existsSync} from "fs";
import {UciEngine} from "./engineManager.js";
import {LichessBot} from "./lichessBot.js";
import {db} from "../db/db.js";

const app = express();

app.use(cors({
    origin: ["http://localhost:5173", "http://127.0.0.1:5173"],
}));

app.use(express.json());

const PROD_PATH = path.join(import.meta.dir, "myengine");
const DEV_PATH = path.resolve(import.meta.dir, "..", "..", "engines", "myengine", "build", "myengine");
const STOCKFISH_PATH = "/usr/bin/stockfish";

let selectedPath = null;

if (existsSync(PROD_PATH)) {
    console.log("✅ Running in Production Mode (Local Binary)");
    selectedPath = PROD_PATH;
} else if (existsSync(DEV_PATH)) {
    console.log("⚠️  Running in Dev Mode (External Binary)");
    selectedPath = DEV_PATH;
} else {
    const FIX_DEV_PATH = path.resolve(import.meta.dir, "..", "..", "..", "engines", "myengine", "build", "myengine");

    if (existsSync(FIX_DEV_PATH)) {
        console.log("⚠️  Running in Dev Mode (Deep Nested Fallback)");
        selectedPath = FIX_DEV_PATH;
    }
}
export const MY_ENGINE_PATH = selectedPath;

console.log(`♟️  Engine Path: ${MY_ENGINE_PATH}`);

if (!MY_ENGINE_PATH) {
    console.error("❌ CRITICAL: Could not find chess engine binary!");
    console.error(`   Checked Prod: ${PROD_PATH}`);
    console.error(`   Checked Dev:  ${DEV_PATH}`);
    process.exit(1);
}

const mainEngine = new UciEngine(MY_ENGINE_PATH, "PrimaryEngine");
const lichessEngine = new UciEngine(MY_ENGINE_PATH, "LichessEngine");
mainEngine.start();

let lichessBotInstance = null;


app.get("/api/health", (req, res) => {
    res.json({status: "ok", engine: mainEngine.ready ? "ready" : "starting"});
});

app.post("/api/engine/analysis", async (req, res) => {
    try {
        const {fen, depth = 10} = req.body;
        if (!fen) return res.status(400).json({error: "FEN required"});

        const bestMove = await mainEngine.bestMoveFromFen(fen, depth);
        res.json({bestMove, depth});
    } catch (err) {
        console.error("Analysis Error:", err);
        res.status(500).json({error: err.message});
    }
});

app.post("/api/engine/make-move", async (req, res) => {
    try {
        const {fen, move} = req.body;
        const result = await mainEngine.makeMove(fen, move);
        res.json({status: "moved", result});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.post("/api/engine/go", async (req, res) => {
    try {
        const {fen, moves, options} = req.body;
        await mainEngine.position(fen || "startpos", moves || []);
        const bestMove = await mainEngine.go(options || {depth: 10});

        res.json({bestMove});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.post("/api/engine/reset", async (req, res) => {
    await mainEngine.uciNewGame();
    res.json({status: "reset_complete"});
});

app.post("/api/engine/print", async (req, res) => {
    const {fen} = req.body;
    await mainEngine.printBoard(fen);
    res.json({status: "printed_to_console"});
});

app.post("/api/arena/trigger", async (req, res) => {
    const {games = 10, depth = 6} = req.body;

    console.log(`[Arena] Starting match: MyEngine vs Stockfish (${games} games)`);

    const p1 = new UciEngine(MY_ENGINE_PATH, "MyEngine_Arena");
    const p2 = new UciEngine(STOCKFISH_PATH, "Stockfish_Arena");

    UciEngine.playMatch(p1, p2, {depth, maxMoves: 200})
    .then(() => console.log("[Arena] Match finished"))
    .catch(err => console.error("[Arena] Match failed", err));

    res.json({
        status: "started",
        message: "Match started in background. Check server logs.",
    });
});

app.post("/api/engine/bench", async (req, res) => {
    try {
        const { mode = "depth", depth = 9, timeLimit = 1000, evalTime = 2000 } = req.body;

        console.log(`Starting benchmark [Mode: ${mode}, Depth: ${depth}, Time: ${timeLimit}ms]...`);

        const results = await mainEngine.bench({
            mode,
            depth,
            timeLimit,
            evalTime
        });

        console.log("Benchmark results:", results);
        res.json({ status: "success", data: results });

    } catch (err) {
        if (err.message === "Cancelled by user") {
            console.log("Benchmark request cancelled cleanly.");
            return res.json({
                status: "cancelled",
                message: "Benchmark was stopped by the user."
            });
        }

        console.error("Benchmark failed:", err);
        res.status(500).json({ error: err.message });
    }
});

app.post("/api/engine/cancel", async (req, res) => {
    try {
        await mainEngine.cancel();
        res.json({ status: "success", message: "Benchmark cancelled" });
    } catch (err) {
        console.error("Cancel failed:", err);
        res.status(500).json({ error: err.message });
    }
});

app.post("/api/lichess/start", async (req, res) => {
    if (lichessBotInstance) {
        return res.status(400).json({ error: "Bot is already running." });
    }

    const token = process.env.lichess_api_token;
    if (!token) {
        return res.status(400).json({ error: "Missing Lichess Token" });
    }

    try {
        lichessBotInstance = new LichessBot(token, lichessEngine);

        lichessBotInstance.start().catch(err => {
            console.error("Lichess Bot Crashed:", err);
            lichessBotInstance = null;
        });

        res.json({ status: "success", message: "Lichess Bot started and listening for events." });

    } catch (err) {
        console.error("Failed to start Lichess Bot:", err);
        res.status(500).json({ error: err.message });
    }
});

/**
 * STOP Lichess Bot
 */
app.post("/api/lichess/stop", async (req, res) => {
    if (!lichessBotInstance) {
        return res.json({ status: "ignored", message: "Bot was not running." });
    }

    try {
        await lichessEngine.stop();
        lichessBotInstance = null;

        res.json({ status: "success", message: "Lichess Bot stopped." });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * STATUS Check
 */
app.get("/api/lichess/status", (req, res) => {
    res.json({
        running: !!lichessBotInstance,
        profile: lichessBotInstance ? lichessBotInstance.botProfile : null,
        activeGames: lichessBotInstance ? Array.from(lichessBotInstance.activeGames) : []
    });
});

/**
 * CREATE Open Challenge
 */
app.post("/api/lichess/challenge/open", async (req, res) => {
    if (!lichessBotInstance) return res.status(400).json({ error: "Bot not running" });
    const { limit = 180, increment = 0 } = req.body;
    try {
        const result = await lichessBotInstance.createOpenChallenge(limit, increment);
        res.json({ status: "success", data: result });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * CREATE AI Challenge (Stockfish)
 */
app.post("/api/lichess/challenge/ai", async (req, res) => {
    if (!lichessBotInstance) return res.status(400).json({ error: "Bot not running" });
    const { level = 1, limit = 180, increment = 0 } = req.body;
    try {
        const result = await lichessBotInstance.createAiChallenge(level, limit, increment);
        res.json({ status: "success", data: result });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * CHALLENGE WEAKEST Bot
 */
app.post("/api/lichess/challenge/weakest", async (req, res) => {
    if (!lichessBotInstance) return res.status(400).json({ error: "Bot not running" });
    const { limit = 180, increment = 0 } = req.body;
    try {
        const result = await lichessBotInstance.huntWeakestBot(limit, increment);
        res.json(result);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.post("/api/start_game", (req, res) => {
    const { player1_id, player2_id } = req.body;
    console.log(`[Local Game] Request to start game: ${player1_id} vs ${player2_id}`);

    res.json({
        status: "started",
        gameId: "local_game_" + Date.now(),
        message: "Local game logic not yet fully implemented, but request received."
    });
});

const PORT = process.env.PORT || 8000;
app.listen(PORT, () => {
    console.log(`Backend listening on http://localhost:${PORT}`);
});
