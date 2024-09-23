#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

class Board {
public:
    Board();
    void initializeBoard();
    void printBoard() const;
    bool makeMove(const std::string& move);
    std::string getBoardState() const;

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

    // Move generation functions for all pieces
    uint64_t generatePawnMoves(int square, bool isWhite) const;
    uint64_t generateKnightMoves(int square) const;
    uint64_t generateKingMoves(int square) const;

private:
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKings;
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKings;

    // Helper functions for bit manipulation
    bool isBitSet(uint64_t bitboard, int position) const;
    void setBit(uint64_t& bitboard, int position);
    void clearBit(uint64_t& bitboard, int position);

    // Helper to convert board position (rank, file) to bit position (0-63)
    int getPosition(int rank, int file) const { return rank * 8 + file; }

    // Magic bitboard methods for rooks, bishops, and queens
    uint64_t generateRookAttacks(int square, uint64_t blockers) const;
    uint64_t generateBishopAttacks(int square, uint64_t blockers) const;
    uint64_t generateQueenAttacks(int square, uint64_t blockers) const;

    // Magic numbers for rooks and bishops
    static const uint64_t rookMagicNumbers[64];
    static const uint64_t bishopMagicNumbers[64];

    // Precomputed attack tables for rooks, bishops, and queens
    uint64_t rookAttackTable[64][4096];  // 4096 possible blocker configurations for rooks
    uint64_t bishopAttackTable[64][512]; // 512 possible blocker configurations for bishops
    uint64_t queenAttackTable[64][4096]; // Rook + bishop moves combined for queens

    // Precompute the sliding piece attacks
    void initializeMagicBitboards();

    // Helper to generate blocker masks
    uint64_t generateBlockerMask(int square, const std::vector<int>& directions) const;

    // Calculate attacks for sliding pieces (rooks, bishops, queens) given a blocker mask
    uint64_t calculateRookAttack(int square, uint64_t blockers) const;
    uint64_t calculateBishopAttack(int square, uint64_t blockers) const;
    uint64_t calculateQueenAttack(int square, uint64_t blockers) const;
};

#endif // BOARD_H
