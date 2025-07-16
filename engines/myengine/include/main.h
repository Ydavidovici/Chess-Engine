// include/main.h
#pragma once

#include <string>
#include <vector>

#include "board.h"
#include "search.h"
#include "evaluator.h"
#include "types.h"
#include "move.h"
#include "transpositionTable.h"
#include "timeman.h"

struct PlaySettings {
    int depth           = 6;
    int tt_size_mb      = 64;
    int time_left_ms    = 0;
    int increment_ms    = 0;
    int moves_to_go     = 30;
};

struct GameData {
    std::vector<std::string> moves;
};

class Engine {
public:
    Engine();
    ~Engine();

    // Position control
    void reset();
    bool setPosition(const std::string &fen);
    std::string getFEN() const;

    // Evaluation
    int evaluateCurrentPosition() const;

    // Move control
    bool applyMove(const std::string &uci);
    bool undoMove();
    std::vector<std::string> legalMoves() const;

    // Engine play + profiling
    std::string playMove(const PlaySettings &settings);

    // Game state
    bool isGameOver() const;
    GameData getGameData() const;

private:
    Board                 board_;
    std::vector<std::string> history_;
};