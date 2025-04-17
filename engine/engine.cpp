// engine.cpp
#include "engine.h"
#include "board.h"
#include "evaluator.h"
#include <iostream>

Engine::Engine() {
    board = Board();
    evaluator = Evaluator();
}

void Engine::newGame() {
    board.initialize();
}

void Engine::printBoard() const {
    for (auto& line : board.getBoardState()) {
        std::cout << line << "\n";
    }
}

bool Engine::makeMove(const Move& m, Color c) {
    if (!board.makeMove(m, c)) {
        std::cerr << "Invalid move: " << m.toString() << "\n";
        return false;
    }
    return true;
}

int Engine::evaluateBoard() const {
    return evaluator.evaluate(board);
}

Move Engine::findBestMove(int maxDepth, Color side) {
    bool maximizing = (side == Color::WHITE);
    return evaluator.iterativeDeepening(board, maxDepth, maximizing);
}