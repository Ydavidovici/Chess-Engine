#pragma once
#include "board.h"
#include "evaluator.h"
#include "search.h"
#include "transpositionTable.h"
#include "timeManager.h"
#include <vector>
#include <string>

struct PlaySettings {
    int depth;
    int time_left_ms;
    int increment_ms;
    int moves_to_go;
    int tt_size_mb;
};

struct GameData {
    std::vector<std::string> moves;
};

class Engine {
public:
    Engine();
    ~Engine();

    void reset();
    bool setPosition(const std::string &fen);

    std::string playMove(const PlaySettings &settings);

    std::string getFEN() const;
    int evaluateCurrentPosition();
    bool applyMove(const std::string &uci);
    bool undoMove();
    std::vector<std::string> legalMoves() const;
    bool isGameOver() const;
    GameData getGameData() const;

    // --- NEW: Getters for Benchmarking ---
    // These allow the Bench class to access internal components
    Search& getSearch() { return searcher; }
    Evaluator& getEvaluator() { return evaluator; }
    Board& getBoard() { return board; }
    const Board& getBoard() const { return board; }

private:
    Board board;
    std::vector<std::string> history;

    TranspositionTable tt;
    Evaluator evaluator;
    Search searcher;
};