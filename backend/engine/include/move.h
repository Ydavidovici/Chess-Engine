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
    // Constructors
    Move(const std::string& moveStr, MoveType type = NORMAL);
    Move(int start, int end, MoveType type = NORMAL);

    // Getters
    int getStartPosition() const;
    int getEndPosition() const;
    std::string getMove() const;
    int getStartRow() const;
    int getStartCol() const;
    int getEndRow() const;
    int getEndCol() const;
    MoveType getMoveType() const;
    char getPromotionPiece() const;

    // Setters
    void setMoveType(MoveType type);
    void setPromotionPiece(char promotionPiece);

    // Validation
    bool isValid() const;
    bool isValidPromotion() const;
    bool isValidCastling() const;
    bool isCapture() const;

private:
    int startPosition;    // 0-63 index
    int endPosition;      // 0-63 index
    MoveType moveType;
    char promotionPiece;  // 'Q', 'R', 'B', 'N' for promotions

    // Helper functions
    bool isInBounds(int position) const;
    bool isValidMoveType() const;
};

#endif // MOVE_H
