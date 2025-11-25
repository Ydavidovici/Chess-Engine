DROP TABLE IF EXISTS game_moves;
DROP TABLE IF EXISTS games;
DROP TABLE IF EXISTS player_stats;

CREATE TABLE IF NOT EXISTS player_stats (
    player_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE,
    wins INT DEFAULT 0,
    losses INT DEFAULT 0,
    draws INT DEFAULT 0
);

CREATE TABLE IF NOT EXISTS games (
     game_id SERIAL PRIMARY KEY,
     start_time TIMESTAMP NOT NULL,
     end_time TIMESTAMP,
     result TEXT,
     player1_name VARCHAR(100) NOT NULL,
     player2_name VARCHAR(100) NOT NULL
);


CREATE TABLE IF NOT EXISTS game_moves (
    game_id INT,
    move_number INT,
    move_notation TEXT,
    PRIMARY KEY (game_id, move_number),
    FOREIGN KEY (game_id) REFERENCES games(game_id)
);
