#ifndef PIECE_H
#define PIECE_H

class Piece {
public:
    enum Type { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
    enum Color { WHITE, BLACK };

    Piece(Type type, Color color);
    char getSymbol() const;

    // Add these methods to get piece type and color
    Type getType() const;
    Color getColor() const;

private:
    Type type;
    Color color;
};

#endif // PIECE_H
