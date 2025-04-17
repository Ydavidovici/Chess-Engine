// board.h
#pragma once
#include <vector>
#include <string>
#include "move.h"

class Board {
public:
    Board();
    void initialize();
    void loadFEN(const std::string& fen);
    std::vector<std::string> getBoardState() const;
    bool makeMove(const Move& m, Color c);
    std::vector<Move> generateLegalMoves(Color c) const;

    // --- Add these accessors ---
    uint64_t getWhitePawns()   const { return whitePawns; }
    uint64_t getWhiteKnights() const { return whiteKnights; }
    uint64_t getWhiteBishops() const { return whiteBishops; }
    uint64_t getWhiteRooks()   const { return whiteRooks; }
    uint64_t getWhiteQueens()  const { return whiteQueens; }
    uint64_t getWhiteKings()   const { return whiteKings; }
    uint64_t getBlackPawns()   const { return blackPawns; }
    uint64_t getBlackKnights() const { return blackKnights; }
    uint64_t getBlackBishops() const { return blackBishops; }
    uint64_t getBlackRooks()   const { return blackRooks; }
    uint64_t getBlackQueens()  const { return blackQueens; }
    uint64_t getBlackKings()   const { return blackKings; }

private:
    uint64_t whitePawns, whiteKnights, whiteBishops,
            whiteRooks, whiteQueens, whiteKings;
    uint64_t blackPawns, blackKnights, blackBishops,
            blackRooks, blackQueens, blackKings;

    static int pos(int rank, int file) { return rank*8 + file; }
    bool bit(uint64_t bb, int sq) const { return (bb>>sq)&1; }
    void set(uint64_t& bb, int sq) { bb |= (1ULL<<sq); }
    void clear(uint64_t& bb, int sq) { bb &= ~(1ULL<<sq); }
};