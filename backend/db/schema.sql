

-- Create a table for storing player stats
CREATE TABLE player_stats (
                              player_id SERIAL PRIMARY KEY,
                              name VARCHAR(100) NOT NULL,
                              wins INT DEFAULT 0,
                              losses INT DEFAULT 0,
                              draws INT DEFAULT 0
);

-- Create a table for games
CREATE TABLE games (
                       game_id SERIAL PRIMARY KEY,
                       start_time TIMESTAMP NOT NULL,
                       end_time TIMESTAMP,
                       result TEXT,
                       player1_id INT,
                       player2_id INT,
                       FOREIGN KEY (player1_id) REFERENCES player_stats(player_id),
                       FOREIGN KEY (player2_id) REFERENCES player_stats(player_id)
);

-- Create a table for storing game moves
CREATE TABLE game_moves (
                            game_id INT,
                            move_number INT,
                            move_notation TEXT,
                            PRIMARY KEY (game_id, move_number),
                            FOREIGN KEY (game_id) REFERENCES games(game_id)
);

-- Add indexes for frequently queried columns
CREATE INDEX idx_games_start_time ON games(start_time);
CREATE INDEX idx_game_moves_game_id ON game_moves(game_id);
CREATE INDEX idx_player_stats_name ON player_stats(name);
