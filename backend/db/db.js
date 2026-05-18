import {Database} from "bun:sqlite";
import {drizzle} from "drizzle-orm/bun-sqlite";
import * as schema from "./schema.js";

const sqlite = new Database("myengine.db");

// Ensure the base tables exist
sqlite.run(`
    CREATE TABLE IF NOT EXISTS players (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE
    )
`);

sqlite.run(`
    CREATE TABLE IF NOT EXISTS games (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        white_id INTEGER NOT NULL REFERENCES players(id),
        black_id INTEGER NOT NULL REFERENCES players(id),
        result TEXT,
        termination TEXT,
        started_at TEXT DEFAULT CURRENT_TIMESTAMP,
        finished_at TEXT
    )
`);

sqlite.run(`
    CREATE TABLE IF NOT EXISTS game_moves (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
        ply INTEGER NOT NULL,
        uci TEXT NOT NULL,
        fen_after TEXT
    )
`);

sqlite.run(`CREATE INDEX IF NOT EXISTS game_idx ON game_moves(game_id)`);

// Idempotent migrations — each ALTER is wrapped so re-runs are safe
const migrations = [
    "ALTER TABLE games ADD COLUMN source TEXT DEFAULT 'local'",
    "ALTER TABLE games ADD COLUMN lichess_game_id TEXT",
    "ALTER TABLE games ADD COLUMN variant TEXT",
    "ALTER TABLE games ADD COLUMN rated INTEGER",
    "ALTER TABLE games ADD COLUMN time_control TEXT",
    "ALTER TABLE games ADD COLUMN white_rating INTEGER",
    "ALTER TABLE games ADD COLUMN black_rating INTEGER",
    "ALTER TABLE games ADD COLUMN opening_eco TEXT",
    "ALTER TABLE games ADD COLUMN opening_name TEXT",
];

for (const stmt of migrations) {
    try {
        sqlite.run(stmt);
    } catch (_) {
        // Column already exists — ignore
    }
}

// Unique index on lichess_game_id (idempotent)
try {
    sqlite.run("CREATE UNIQUE INDEX IF NOT EXISTS lichess_game_id_idx ON games(lichess_game_id)");
} catch (_) {}

export const db = drizzle(sqlite, {schema});
