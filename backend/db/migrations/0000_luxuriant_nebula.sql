CREATE TABLE `game_moves` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`game_id` integer NOT NULL,
	`ply` integer NOT NULL,
	`uci` text NOT NULL,
	`fen_after` text
);
--> statement-breakpoint
CREATE TABLE `games` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`white_id` integer NOT NULL,
	`black_id` integer NOT NULL,
	`result` text,
	`started_at` text,
	`finished_at` text
);
--> statement-breakpoint
CREATE TABLE `players` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`name` text NOT NULL
);
--> statement-breakpoint
CREATE UNIQUE INDEX `players_name_unique` ON `players` (`name`);