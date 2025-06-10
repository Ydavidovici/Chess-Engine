// engine.cpp
#include "engine.h"
#include "board.h"
#include "evaluator.h"
#include <iostream>

Engine::Engine() {
    // Initialize a fresh board and evaluator
    board = Board();
    evaluator = Evaluator();
}

void Engine::newGame() {
    // Reset to the starting position
    board.initialize();
}

void Engine::printBoard() const {
    // Print each rank of the board to stdout
    for (const auto& line : board.getBoardState()) {
        std::cout << line << "\n";
    }
}

bool Engine::makeMove(const Move& m, Color c) {
    // Attempt to apply the move; log and return false if invalid
    if (!board.makeMove(m, c)) {
        std::cerr << "Invalid move: " << m.toString() << "\n";
        return false;
    }
    return true;
}

int Engine::evaluateBoard() const {
    // Return static evaluation of current position
    return evaluator.evaluate(board);
}

Move Engine::findBestMove(int maxDepth, Color side) {
    // Run iterative deepening and return the best move found
    bool maximizing = (side == Color::WHITE);
    return evaluator.iterativeDeepening(board, maxDepth, maximizing);
}