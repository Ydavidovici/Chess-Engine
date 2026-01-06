import express from "express";
import cors from "cors";
import path from "node:path";
import {UciEngine} from "./engineManager.js";
import {db} from "../db/db.js";

const app = express();

app.use(cors({
    origin: ["http://localhost:5173", "http://127.0.0.1:5173"],
}));

app.use(express.json());

const MY_ENGINE_PATH = path.join(import.meta.dir, "engines", "myengine", "build", "myengine");
const STOCKFISH_PATH = "/usr/bin/stockfish";

const mainEngine = new UciEngine(MY_ENGINE_PATH, "PrimaryEngine");
mainEngine.start();


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

const PORT = process.env.PORT || 8000;
app.listen(PORT, () => {
    console.log(`Backend listening on http://localhost:${PORT}`);
});
