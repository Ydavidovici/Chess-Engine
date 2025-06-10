// engine/src/engine.cpp
#include "engine.h"
#include <iostream>

Engine::Engine() {}

void Engine::printBoard() const {
    // Use toFEN instead of getBoardState
    std::cout << board.toFEN() << "\n";
}

bool Engine::makeMove(const Move& m, Color c) {
    // Ensure it's our turn
    if (board.sideToMove() != c) return false;
    // Single-arg makeMove
    bool ok = board.makeMove(m);
    if (ok) lastMove = m;
    return ok;
}

Move Engine::getLastMove() const {
    return lastMove;
}


