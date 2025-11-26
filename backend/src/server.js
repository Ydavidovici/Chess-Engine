import express from "express";
import cors from "cors";

const app = express();

app.use(cors({
    origin: ["http://localhost:5173", "http://127.0.0.1:5173"],
}));
app.use(express.json());

app.get("/api/health", async (req, res) => {
    try {
        res.json({
            status: "ok",
        });
    } catch (err) {
        console.error("Health failed:", err);
        res.status(500).json({status: "error", error: String(err)});
    }
});

// app.post("/players", async (req, res) => {
//   try {
//     const { name } = req.body;
//     if (!name) {
//       return res.status(400).json({ error: "name is required" });
//     }
//
//     const result = await db.insert(players).values({ name }).run();
//     const lastId = result.lastInsertRowid;
//
//     const rows = await db
//       .select()
//       .from(players)
//       .where(eq(players.id, lastId));
//
//     const player = rows[0] || null;
//
//     res.status(201).json(player);
//   } catch (err) {
//     console.error("Create player failed:", err);
//     res.status(500).json({ error: String(err) });
//   }
// });

const PORT = process.env.PORT;

app.listen(PORT, () => {
    console.log(`Backend listening on http://localhost:${PORT}`);
});
