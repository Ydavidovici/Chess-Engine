#include "engine.h"
#include "player.h"
#include <iostream>

Engine::Engine()
        : whitePlayer(Player::Color::WHITE),  // Correctly scoped Color enum
          blackPlayer(Player::Color::BLACK)  // Correctly scoped Color enum
{
    board.initializeBoard();
}


void Engine::initialize() {
    board.initializeBoard();
}

std::string Engine::getBoardState() const {
    return board.getBoardState();
}

bool Engine::makeMove(const Move& move) {
    if (!board.makeMove(move.getMove())) {
        std::cerr << "Invalid move: " << move.getMove() << std::endl;
        return false;
    }
    return true;
}

int Engine::evaluateBoard() const {
    return evaluator.evaluate(board);
}
