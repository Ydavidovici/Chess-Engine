// engine.h
#pragma once
#include "board.h"
#include "move.h"
#include "evaluator.h"

class Engine {
public:
    Engine();

    // Start a new game (reset board to initial position)
    void newGame();

    // Print the current board to stdout
    void printBoard() const;

    // Attempt to make a move; returns false if illegal
    bool makeMove(const Move& m, Color c);

    // Evaluate current board statically
    int evaluateBoard() const;

    // Find best move up to given depth
    Move findBestMove(int maxDepth, Color side);

private:
    Board       board;
    Evaluator   evaluator;
};