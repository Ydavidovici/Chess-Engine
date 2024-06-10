// backend/engine/src/engine.cpp

#include "engine.h"
#include "player.h"

Engine::Engine() : whitePlayer(Player::WHITE), blackPlayer(Player::BLACK) {}

void Engine::initialize() {
    board.initializeBoard();
}

bool Engine::makeMove(const std::string& move) {
    // For simplicity, assume the move string is in algebraic notation and is valid
    // Implement move validation and application logic here
    return true;
}

std::string Engine::getBoardState() const {
    return board.getBoardState();
}

int Engine::evaluateBoard() const {
    return evaluator.evaluate(board);
}
