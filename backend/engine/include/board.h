#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <string>
#include <vector>
#include "move.h"

class Board {
public:
    Board();
    void initializeBoard();
    std::string getBoardState() const;
    bool makeMove(const Move& move);

    // Public accessors for bitboards
    uint64_t getWhitePawns() const { return whitePawns; }
    uint64_t getWhiteKnights() const { return whiteKnights; }
    uint64_t getWhiteBishops() const { return whiteBishops; }
    uint64_t getWhiteRooks() const { return whiteRooks; }
    uint64_t getWhiteQueens() const { return whiteQueens; }
    uint64_t getWhiteKings() const { return whiteKings; }

    uint64_t getBlackPawns() const { return blackPawns; }
    uint64_t getBlackKnights() const { return blackKnights; }
    uint64_t getBlackBishops() const { return blackBishops; }
    uint64_t getBlackRooks() const { return blackRooks; }
    uint64_t getBlackQueens() const { return blackQueens; }
    uint64_t getBlackKings() const { return blackKings; }

    uint64_t generatePawnMoves(int square, bool isWhite) const;
    uint64_t generateKnightMoves(int square) const;
    uint64_t generateKingMoves(int square) const;

private:
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKings;
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKings;

    bool isBitSet(uint64_t bitboard, int position) const;
    void setBit(uint64_t& bitboard, int position);
    void clearBit(uint64_t& bitboard, int position);
    int getPosition(int rank, int file) const;
};

#endif // BOARD_H
