// engine/include/board.h
#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <string>
#include "move.h"
#include "types.h"

class Board {
public:
    // Piece types (used to index our bitboard arrays)
    enum PieceIndex { PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING, PieceTypeCount };

    Board();
    void initialize();
    void loadFEN(const std::string& fen);
    std::string toFEN() const;

    std::vector<Move> generatePseudoMoves() const;
    std::vector<Move> generateLegalMoves() const;
    bool makeMove(const Move& m);
    void unmakeMove();

    // Inspectors
    uint64_t occupancy(Color c) const;
    uint64_t pieceBB(Color c, PieceIndex pi) const;
    Color sideToMove() const { return side_to_move; }

private:
    // Bitboards for each piece type
    std::array<uint64_t, PieceTypeCount> whiteBB{};
    std::array<uint64_t, PieceTypeCount> blackBB{};

    // Game state
    Color   side_to_move;
    uint8_t castling_rights;    // bits: Wk, WQ, bk, bQ
    int     en_passant_square;  // -1 if none
    int     halfmove_clock;
    int     fullmove_number;

    // Undo history entry for unmakeMove
    struct Undo {
        uint8_t      castling_rights;
        int          en_passant_square;
        int          halfmove_clock;
        int          fullmove_number;
        Move         move;
        PieceIndex   moved_piece;
        PieceIndex   captured_piece;
        bool         is_pawn_double;
        bool         is_castling;
        int          castling_rook_from;
        int          castling_rook_to;
    };
    std::vector<Undo> history;

    // Bitboard helpers
    static bool inBounds(int sq) { return sq >= 0 && sq < 64; }
    static inline void setBit(uint64_t& bb, int sq)   { bb |= (1ULL << sq); }
    static inline void clearBit(uint64_t& bb, int sq) { bb &= ~(1ULL << sq); }
    static inline bool testBit(uint64_t bb, int sq)   { return (bb >> sq) & 1ULL; }
};
