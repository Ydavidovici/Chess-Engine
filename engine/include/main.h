/* include/main.h */
#pragma once
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "search.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "timeman.h"
#include "types.h"

// Settings for a single engine move
struct PlaySettings {
    int depth = 6;
    size_t tt_size_mb = 64;          // megabytes          // megabytes
    uint64_t time_left_ms = 0;       // 0 = ignore timing, fixed-depth
    uint64_t increment_ms = 0;
    int moves_to_go = 30;
};

// Summary of a played game
struct GameData {
    std::vector<std::string> moves;    // UCI moves in order
};

class Engine {
public:
    Engine();

    // Apply an incoming move (uci). Returns true if legal
    bool applyMove(const std::string &uci);

    // Search & play one move, appending to history, then return UCI
    std::string playMove(const PlaySettings &settings);

    // Query if the game is over
    bool isGameOver() const;

    // Retrieve the full move history once game ends
    GameData getGameData() const;

private:
    Board board_;
    std::vector<std::string> history_;
};