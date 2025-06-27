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

    // --- endgame detection ---
    bool inCheck(Color c) const;
    bool hasLegalMoves(Color c) const;
    bool isCheckmate(Color c) const;
    bool isStalemate(Color c) const;
    bool isFiftyMoveDraw() const;
    bool isThreefoldRepetition() const;
    bool isInsufficientMaterial() const;

    // Inspectors
    uint64_t occupancy(Color c) const;
    uint64_t pieceBB(Color c, PieceIndex pi) const;
    Color sideToMove() const { return side_to_move; }

    // Zobrist key for hashing
    uint64_t zobristKey() const { return zobrist_; }

private:
    std::array<uint64_t, PieceTypeCount> whiteBB{};
    std::array<uint64_t, PieceTypeCount> blackBB{};

    Color   side_to_move;
    uint8_t castling_rights;
    int     en_passant_square;
    int     halfmove_clock;
    int     fullmove_number;

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
    std::vector<std::string> positionHistory;

    // Bitboard helpers
    static bool inBounds(int sq) { return sq >= 0 && sq < 64; }
    static inline void setBit(uint64_t& bb, int sq)   { bb |= (1ULL << sq); }
    static inline void clearBit(uint64_t& bb, int sq) { bb &= ~(1ULL << sq); }
    static inline bool testBit(uint64_t bb, int sq)   { return (bb >> sq) & 1ULL; }

    bool isSquareAttacked(int sq, Color by) const;
    int  findKing(Color c) const;

    // Zobrist hash state
    uint64_t zobrist_ = 0;
};