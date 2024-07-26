-- Drop existing tables if they exist
DROP TABLE IF EXISTS game_moves;
DROP TABLE IF EXISTS games;
DROP TABLE IF EXISTS player_stats;

-- Create a table for storing player stats
CREATE TABLE IF NOT EXISTS player_stats (
                                            player_id SERIAL PRIMARY KEY,
                                            name VARCHAR(100) NOT NULL UNIQUE,
    wins INT DEFAULT 0,
    losses INT DEFAULT 0,
    draws INT DEFAULT 0
    );

-- Create a table for games
CREATE TABLE IF NOT EXISTS games (
                                     game_id SERIAL PRIMARY KEY,
                                     start_time TIMESTAMP NOT NULL,
                                     end_time TIMESTAMP,
                                     result TEXT,
                                     player1_name VARCHAR(100) NOT NULL,
    player2_name VARCHAR(100) NOT NULL
    );

-- Create a table for storing game moves
CREATE TABLE IF NOT EXISTS game_moves (
                                          game_id INT,
                                          move_number INT,
                                          move_notation TEXT,
                                          PRIMARY KEY (game_id, move_number),
    FOREIGN KEY (game_id) REFERENCES games(game_id)
    );

-- Add indexes for frequently queried columns
CREATE INDEX IF NOT EXISTS idx_games_start_time ON games(start_time);
CREATE INDEX IF NOT EXISTS idx_game_moves_game_id ON game_moves(game_id);
CREATE INDEX IF NOT EXISTS idx_player_stats_name ON player_stats(name);
