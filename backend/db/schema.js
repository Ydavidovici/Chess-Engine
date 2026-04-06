import {sqliteTable, integer, text} from "drizzle-orm/sqlite-core";

export const players = sqliteTable("players", {
    id: integer("id").primaryKey({autoIncrement: true}),
    name: text("name").notNull().unique(),
});

export const games = sqliteTable("games", {
    id: integer("id").primaryKey({autoIncrement: true}),
    whiteId: integer("white_id").notNull(),
    blackId: integer("black_id").notNull(),
    result: text("result"),
    startedAt: text("started_at"),
    finishedAt: text("finished_at"),
});

export const gameMoves = sqliteTable("game_moves", {
    id: integer("id").primaryKey({autoIncrement: true}),
    gameId: integer("game_id").notNull(),
    ply: integer("ply").notNull(),
    uci: text("uci").notNull(),
    fenAfter: text("fen_after"),
});
