#include "piece.h"

Piece::Piece(Type type, Color color) : type(type), color(color) {}

char Piece::getSymbol() const {
    switch (type) {
        case PAWN: return color == WHITE ? 'P' : 'p';
        case ROOK: return color == WHITE ? 'R' : 'r';
        case KNIGHT: return color == WHITE ? 'N' : 'n';
        case BISHOP: return color == WHITE ? 'B' : 'b';
        case QUEEN: return color == WHITE ? 'Q' : 'q';
        case KING: return color == WHITE ? 'K' : 'k';
        default: return '?';
    }
}

// New methods to get type and color
Piece::Type Piece::getType() const {
    return type;
}

Piece::Color Piece::getColor() const {
    return color;
}
