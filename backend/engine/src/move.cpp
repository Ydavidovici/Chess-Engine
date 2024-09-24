#include "move.h"

// Constructor for move with optional move type (defaults to NORMAL move)
Move::Move(const std::string& move, MoveType type) : moveType(type), promotionPiece(0) {
    startPosition = (move[1] - '1') * 8 + (move[0] - 'a');
    endPosition = (move[3] - '1') * 8 + (move[2] - 'a');
}

// Constructor for move with start and end positions
Move::Move(int start, int end, MoveType type) : startPosition(start), endPosition(end), moveType(type), promotionPiece(0) {}

// Get start position (0-63 index)
int Move::getStartPosition() const {
    return startPosition;
}

// Get end position (0-63 index)
int Move::getEndPosition() const {
    return endPosition;
}

// Get the move as a string in standard chess notation (e.g., "e2e4")
std::string Move::getMove() const {
    std::string moveStr;
    moveStr += char((startPosition % 8) + 'a');
    moveStr += char((startPosition / 8) + '1');
    moveStr += char((endPosition % 8) + 'a');
    moveStr += char((endPosition / 8) + '1');
    return moveStr;
}

// Get start row (0-7)
int Move::getStartRow() const {
    return startPosition / 8;
}

// Get start column (0-7)
int Move::getStartCol() const {
    return startPosition % 8;
}

// Get end row (0-7)
int Move::getEndRow() const {
    return endPosition / 8;
}

// Get end column (0-7)
int Move::getEndCol() const {
    return endPosition % 8;
}

// Get move type (normal, capture, castling, etc.)
MoveType Move::getMoveType() const {
    return moveType;
}

// Set move type (normal, castling, promotion, etc.)
void Move::setMoveType(MoveType type) {
    moveType = type;
}

// Set the promotion piece for a pawn promotion (e.g., 'Q' for Queen, 'N' for Knight)
void Move::setPromotionPiece(char promotionPiece) {
    this->promotionPiece = promotionPiece;
}

// Get the promotion piece (returns 0 if not a promotion move)
char Move::getPromotionPiece() const {
    return promotionPiece;
}

// Check if the move is within valid board bounds
bool Move::isInBounds(int position) const {
    return position >= 0 && position < 64;
}

// Validate if the move type is valid
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

// Validate if the move is legal
bool Move::isValid() const {
    return isInBounds(startPosition) && isInBounds(endPosition) && isValidMoveType();
}

// Validate promotion move (start position must be on the 7th or 2nd rank, end on the 8th or 1st rank)
bool Move::isValidPromotion() const {
    if (moveType == PROMOTION) {
        int startRank = getStartRow();
        int endRank = getEndRow();
        return ((startRank == 6 && endRank == 7) || (startRank == 1 && endRank == 0));
    }
    return false;
}

// Validate castling (ensures castling is between the correct squares)
bool Move::isValidCastling() const {
    if (moveType == CASTLE_KINGSIDE) {
        return (startPosition == 4 && (endPosition == 6 || endPosition == 62)); // e1 -> g1 or e8 -> g8
    } else if (moveType == CASTLE_QUEENSIDE) {
        return (startPosition == 4 && (endPosition == 2 || endPosition == 58)); // e1 -> c1 or e8 -> c8
    }
    return false;
}
