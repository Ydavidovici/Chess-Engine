CREATE TABLE `game_moves` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`game_id` integer NOT NULL,
	`ply` integer NOT NULL,
	`uci` text NOT NULL,
	`fen_after` text,
	FOREIGN KEY (`game_id`) REFERENCES `games`(`id`) ON UPDATE no action ON DELETE cascade
);
--> statement-breakpoint
CREATE INDEX `game_idx` ON `game_moves` (`game_id`);--> statement-breakpoint
CREATE TABLE `games` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`white_id` integer NOT NULL,
	`black_id` integer NOT NULL,
	`result` text,
	`termination` text,
	`started_at` text DEFAULT CURRENT_TIMESTAMP,
	`finished_at` text,
	FOREIGN KEY (`white_id`) REFERENCES `players`(`id`) ON UPDATE no action ON DELETE no action,
	FOREIGN KEY (`black_id`) REFERENCES `players`(`id`) ON UPDATE no action ON DELETE no action
);
--> statement-breakpoint
CREATE TABLE `players` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`name` text NOT NULL
);
--> statement-breakpoint
CREATE UNIQUE INDEX `players_name_unique` ON `players` (`name`);