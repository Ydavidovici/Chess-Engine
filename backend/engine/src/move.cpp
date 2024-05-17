// backend/engine/src/move.cpp

#include "move.h"

Move::Move(const std::string& move) : move(move) {}

std::string Move::getMove() const {
    return move;
}
