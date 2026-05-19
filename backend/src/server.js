import express from "express";
import cors from "cors";
import path from "node:path";
import {EngineManager} from "./engineManager.js";
import {LichessBot} from "./lichessBot.js";
import {db} from "../db/db.js";

const app = express();

app.use(cors({
    origin: ["http://localhost:5173", "http://127.0.0.1:5173"],
}));

app.use(express.json());

const PROD_PATH = path.join(import.meta.dir, "myengine");
const DEV_PATH = path.resolve(import.meta.dir, "../../engines/myengine/build/myengine.exe");
const FIX_DEV_PATH = path.resolve(import.meta.dir, "../../../engines/myengine/build/myengine");
const STOCKFISH_PATH = "/usr/bin/stockfish";

let MY_ENGINE_PATH = null;

if (await Bun.file(PROD_PATH).exists()) {
    console.log("✅ Running in Production Mode (Local Binary)");
    MY_ENGINE_PATH = PROD_PATH;
} else if (await Bun.file(DEV_PATH).exists()) {
    console.log("⚠️  Running in Dev Mode (External Binary)");
    MY_ENGINE_PATH = DEV_PATH;
} else if (await Bun.file(FIX_DEV_PATH).exists()) {
    console.log("⚠️  Running in Dev Mode (Deep Nested Fallback)");
    MY_ENGINE_PATH = FIX_DEV_PATH;
}

if (!MY_ENGINE_PATH) {
    console.error("❌ CRITICAL: Could not find chess engine binary!");
    process.exit(1);
}
console.log(`♟️  Engine Path: ${MY_ENGINE_PATH}`);

const manager = new EngineManager();
let lichessBotInstance = null;

await manager.registerEngine("Main", MY_ENGINE_PATH);
await manager.registerEngine("Lichess", MY_ENGINE_PATH);


app.get("/api/health", (req, res) => {
    const mainEngine = manager.getEngine("Main");
    res.json({status: "ok", engine: mainEngine.ready ? "ready" : "starting"});
});

app.post("/api/engine/analysis", async (req, res) => {
    try {
        const {fen, depth = 10} = req.body;
        if (!fen) return res.status(400).json({error: "FEN required"});

        const mainEngine = manager.getEngine("Main");
        await mainEngine.position(fen);
        const bestMove = await mainEngine.go({depth});

        res.json({bestMove, depth});
    } catch (err) {
        console.error("Analysis Error:", err);
        res.status(500).json({error: err.message});
    }
});

app.post("/api/engine/go", async (req, res) => {
    try {
        const {fen, moves, options} = req.body;
        const mainEngine = manager.getEngine("Main");

        await mainEngine.position(fen || "startpos", moves || []);
        const bestMove = await mainEngine.go(options || {depth: 7});

        res.json({bestMove});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.post("/api/engine/reset", async (req, res) => {
    await manager.getEngine("Main").uciNewGame();
    res.json({status: "reset_complete"});
});

app.post("/api/engine/bench", async (req, res) => {
    try {
        const {mode = "depth", depth = 9, timeLimit = 30000, evalTime = 2000} = req.body;
        console.log(`Starting benchmark [Mode: ${mode}, Depth: ${depth}, Time: ${timeLimit}ms]...`);

        const mainEngine = manager.getEngine("Main");
        const results = await mainEngine.bench({mode, depth, timeLimit, evalTime});

        console.log("Benchmark results:", results);
        res.json({status: "success", data: results});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.post("/api/engine/cancel", async (req, res) => {
    try {
        await manager.shutdownEngine("Main");
        await manager.registerEngine("Main", MY_ENGINE_PATH);
        res.json({status: "success", message: "Engine reset to cancel task."});
    } catch (err) {
        console.error("Cancel failed:", err);
        res.status(500).json({error: err.message});
    }
});

app.post("/api/lichess/start", async (req, res) => {
    if (lichessBotInstance) {
        return res.status(400).json({error: "Bot is already running."});
    }

    const token = process.env.lichess_api_token;
    if (!token) {
        return res.status(400).json({error: "Missing Lichess Token"});
    }

    try {
        const lichessEngine = manager.getEngine("Lichess");
        lichessBotInstance = new LichessBot(token, lichessEngine);

        lichessBotInstance.start().catch(err => {
            console.error("Lichess Bot Crashed:", err);
            lichessBotInstance = null;
        });

        res.json({status: "success", message: "Lichess Bot started and listening for events."});
    } catch (err) {
        console.error("Failed to start Lichess Bot:", err);
        res.status(500).json({error: err.message});
    }
});

app.post("/api/lichess/stop", async (req, res) => {
    if (!lichessBotInstance) {
        return res.json({status: "ignored", message: "Bot was not running."});
    }

    try {
        if (typeof lichessBotInstance.stop === "function") {
            lichessBotInstance.stop();
        }
        lichessBotInstance = null;
        res.json({status: "success", message: "Lichess Bot stopped."});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.get("/api/lichess/status", (req, res) => {
    res.json({
        running: !!lichessBotInstance,
        profile: lichessBotInstance ? lichessBotInstance.botProfile : null,
        activeGames: lichessBotInstance ? Array.from(lichessBotInstance.activeGames) : [],
    });
});

app.post("/api/lichess/challenge/open", async (req, res) => {
    if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
    const {limit = 180, increment = 0} = req.body;
    try {
        const result = await lichessBotInstance.createOpenChallenge(limit, increment);
        res.json({status: "success", data: result});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.post("/api/lichess/challenge/ai", async (req, res) => {
    if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
    const {level = 1, limit = 180, increment = 0} = req.body;
    try {
        const result = await lichessBotInstance.createAiChallenge(level, limit, increment);
        res.json({status: "success", data: result});
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

app.post("/api/lichess/challenge/weakest", async (req, res) => {
    if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
    const {limit = 180, increment = 0} = req.body;
    try {
        const result = await lichessBotInstance.huntWeakestBot(limit, increment);
        res.json(result);
    } catch (err) {
        res.status(500).json({error: err.message});
    }
});

const PORT = process.env.PORT || 8000;
app.listen(PORT, () => {
    console.log(`Backend listening on http://localhost:${PORT}`);
});

process.on("SIGINT", async () => {
    console.log("\n[Server] Shutting down cleanly...");
    await manager.shutdownAll();
    process.exit(0);
});