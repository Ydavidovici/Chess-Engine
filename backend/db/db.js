import { Database } from "bun:sqlite";
import { drizzle } from "drizzle-orm/bun-sqlite";
import { players, games, gameMoves } from "./schema.js";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const sqlite = new Database(path.join(__dirname, "myengine.db"), { create: true });

export const db = drizzle(sqlite, {
  schema: {
    players,
    games,
    gameMoves,
  },
});