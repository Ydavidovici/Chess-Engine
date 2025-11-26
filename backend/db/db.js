import { Database } from "bun:sqlite";
import { drizzle } from "drizzle-orm/bun-sqlite";
import { players, games, gameMoves } from "./schema.js";

const __dirname = import.meta.dirname;
const __filename = import.meta.filename;


const sqlite = new Database(path.join(__dirname, "myengine.db"), { create: true });

export const db = drizzle(sqlite, {
  schema: {
    players,
    games,
    gameMoves,
  },

});
