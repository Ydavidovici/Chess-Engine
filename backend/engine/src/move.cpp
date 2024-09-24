#include "move.h"
#include <cctype>

// Constructor for move with move string (e.g., "e2e4")
Move::Move(const std::string& moveStr, MoveType type) : moveType(type), promotionPiece(0) {
    if (moveStr.length() < 4) {
        moveType = INVALID;
        return;
    }
    startPosition = (moveStr[1] - '1') * 8 + (moveStr[0] - 'a');
    endPosition = (moveStr[3] - '1') * 8 + (moveStr[2] - 'a');
    if (moveStr.length() == 5 && moveType == PROMOTION) {
        promotionPiece = std::toupper(moveStr[4]);
    }
}

// Constructor for move with start and end positions
Move::Move(int start, int end, MoveType type) : startPosition(start), endPosition(end), moveType(type), promotionPiece(0) {}

// Getters
int Move::getStartPosition() const { return startPosition; }
int Move::getEndPosition() const { return endPosition; }

std::string Move::getMove() const {
    std::string moveStr;
    moveStr += char((startPosition % 8) + 'a');
    moveStr += char((startPosition / 8) + '1');
    moveStr += char((endPosition % 8) + 'a');
    moveStr += char((endPosition / 8) + '1');
    if (promotionPiece != 0) {
        moveStr += promotionPiece;
    }
    return moveStr;
}

int Move::getStartRow() const { return startPosition / 8; }
int Move::getStartCol() const { return startPosition % 8; }
int Move::getEndRow() const { return endPosition / 8; }
int Move::getEndCol() const { return endPosition % 8; }

MoveType Move::getMoveType() const { return moveType; }
char Move::getPromotionPiece() const { return promotionPiece; }

// Setters
void Move::setMoveType(MoveType type) { moveType = type; }
void Move::setPromotionPiece(char promotion) { promotionPiece = std::toupper(promotion); }

// Validation
bool Move::isInBounds(int position) const {
    return position >= 0 && position < 64;
}

bool Move::isValidMoveType() const {
    switch (moveType) {
        case NORMAL:
        case CAPTURE:
        case CASTLE_KINGSIDE:
        case CASTLE_QUEENSIDE:
        case PROMOTION:
        case EN_PASSANT:
            return true;
        default:
            return false;
    }
}

bool Move::isValid() const {
    return isInBounds(startPosition) && isInBounds(endPosition) && isValidMoveType();
}

bool Move::isValidPromotion() const {
    if (moveType != PROMOTION) return false;
    int startRank = getStartRow();
    int endRank = getEndRow();
    // White pawn promotes from rank 7 to 8, Black from rank 2 to 1
    return ((startRank == 6 && endRank == 7) || (startRank == 1 && endRank == 0));
}

bool Move::isValidCastling() const {
    if (moveType == CASTLE_KINGSIDE) {
        // e1 to g1 (White) or e8 to g8 (Black)
        return (startPosition == 4 && (endPosition == 6 || endPosition == 62));
    }
    if (moveType == CASTLE_QUEENSIDE) {
        // e1 to c1 (White) or e8 to c8 (Black)
        return (startPosition == 4 && (endPosition == 2 || endPosition == 58));
    }
    return false;
}

bool Move::isCapture() const {
    return moveType == CAPTURE || moveType == EN_PASSANT;
}
