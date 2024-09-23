#ifndef MOVE_H
#define MOVE_H

#include <string>

enum MoveType {
    NORMAL,
    CAPTURE,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    PROMOTION,
    EN_PASSANT,
    INVALID
};

class Move {
public:
    // Constructor for a regular move
    Move(const std::string& move, MoveType type = MoveType::NORMAL);

    // Constructor for a move with start and end positions
    Move(int start, int end, MoveType type = MoveType::NORMAL);

    // Getters for move positions
    int getStartPosition() const;
    int getEndPosition() const;

    // Get move as string notation (e.g., "e2e4")
    std::string getMove() const;

    // Get start/end rows and columns
    int getStartRow() const;
    int getStartCol() const;
    int getEndRow() const;
    int getEndCol() const;

    // Get and set the move type (normal, castling, promotion, etc.)
    MoveType getMoveType() const;
    void setMoveType(MoveType type);

    // Set promotion piece (for pawn promotions)
    void setPromotionPiece(char promotionPiece);
    char getPromotionPiece() const;

    // Move validation
    bool isValid() const;
    bool isValidPromotion() const;
    bool isValidCastling() const;

private:
    int startPosition;
    int endPosition;
    MoveType moveType;
    char promotionPiece; // For pawn promotions, stores the promoted piece (e.g., 'Q', 'R', 'B', 'N')

    // Helper functions for validation
    bool isInBounds(int position) const;
    bool isValidMoveType() const;
};

#endif // MOVE_H
