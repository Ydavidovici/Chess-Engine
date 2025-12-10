import express from "express";
import cors from "cors";
import {UciEngine} from "./engineManager.js";
import {db} from "../db/db.js";

const app = express();

app.use(cors({
    origin: ["http://localhost:5173", "http://127.0.0.1:5173"],
}));

app.use(express.json());

const engine = new UciEngine();
engine.start();

app.get("/api/health", async (request, response) => {
    try {
        response.json({
            status: "ok",
            engine: "ready",
        });
    } catch (err) {
        console.error("health failed:", err);
        response.status(500).json({status: "error", error: String(err)});
    }
});

app.post("/api/engine/best-move", async (request, response) => {
    try {
        const {fen} = request.body;
        if (!fen) {
            return response.json({error: "fen is required"});
        }
        const bestMove = await engine.bestMoveFromFen(fen);
        response.json({bestMove});
    } catch (err) {
        console.error("best-move failed:", err);
        response.status(500).json({error: String(err)});
    }
});

app.post("/api/engine/print-position", async (request, response) => {
    const {fen} = request.body;
    const board = await engine.printBoard(fen)
    response.json(board);
});

app.post("/api/engine/make-move", async (request, response) => {
    const {fen, move} = request.body
    response.json(await engine.makeMove(fen, move))
});

const PORT = process.env.PORT || 8000;
app.listen(PORT, () => {
    console.log(`Backend listening on http://localhost:${PORT}`);
});
