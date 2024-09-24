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
    Move(const std::string& move, MoveType type = MoveType::NORMAL);
    Move(int start, int end, MoveType type = MoveType::NORMAL);

    int getStartPosition() const;
    int getEndPosition() const;
    std::string getMove() const;
    int getStartRow() const;
    int getStartCol() const;
    int getEndRow() const;
    int getEndCol() const;
    MoveType getMoveType() const;
    void setMoveType(MoveType type);
    void setPromotionPiece(char promotionPiece);
    char getPromotionPiece() const;
    bool isValid() const;
    bool isValidPromotion() const;
    bool isValidCastling() const;

private:
    int startPosition;
    int endPosition;
    MoveType moveType;
    char promotionPiece;
    bool isInBounds(int position) const;
    bool isValidMoveType() const;
};

#endif // MOVE_H
