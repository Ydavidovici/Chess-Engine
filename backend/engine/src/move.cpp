#include "move.h"

Move::Move(const std::string& move) {
    startPosition = (move[1] - '1') * 8 + (move[0] - 'a');
    endPosition = (move[3] - '1') * 8 + (move[2] - 'a');
}

int Move::getStartPosition() const {
    return startPosition;
}

int Move::getEndPosition() const {
    return endPosition;
}

std::string Move::getMove() const {
    // Convert back to chess notation (e.g., e2e4)
    std::string move;
    move += char((startPosition % 8) + 'a');
    move += char((startPosition / 8) + '1');
    move += char((endPosition % 8) + 'a');
    move += char((endPosition / 8) + '1');
    return move;
}

int Move::getStartRow() const {
    return startPosition / 8;
}

int Move::getStartCol() const {
    return startPosition % 8;
}

int Move::getEndRow() const {
    return endPosition / 8;
}

int Move::getEndCol() const {
    return endPosition % 8;
}
