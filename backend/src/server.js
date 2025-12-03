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

app.get("/api/health", async (req, res) => {
    try {
        res.json({
            status: "ok",
            engine: "ready",
        });
    } catch (err) {
        console.error("health failed:", err);
        res.status(500).json({status: "error", error: String(err)});
    }
});

app.post("/api/engine/best-move", async (req, res) => {
    try {
        const {fen} = req.body;
        if (!fen) {
            return res.json({error: "fen is required"});
        }
        const bestMove = await engine.bestMoveFromFen(fen);
        res.json({bestMove});
    } catch (err) {
        console.error("best-move failed:", err);
        res.status(500).json({error: String(err)});
    }
});

const PORT = process.env.PORT || 8000;
app.listen(PORT, () => {
    console.log(`Backend listening on http://localhost:${PORT}`);
});
