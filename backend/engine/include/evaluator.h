#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "board.h"
#include "move.h"
#include <unordered_map>
#include <vector>
#include <cstdint>

// Structure to store transposition table entries
struct HashEntry {
    uint64_t zobristHash;  // Zobrist hash of the position
    int depth;             // Search depth
    int score;             // Evaluation score
    int flag;              // EXACT, ALPHA, BETA
};

enum NodeType { EXACT, ALPHA, BETA };

// Structure for Piece-Square Tables
struct PieceSquareTables {
    int pawn_table[64];
    int knight_table[64];
    int bishop_table[64];
    int rook_table[64];
    int queen_table[64];
    int king_table_mg[64];  // Middlegame King table
    int king_table_eg[64];  // Endgame King table

    void initialize();  // Function to initialize piece-square tables
};

class Evaluator {
public:
    Evaluator();

    // Evaluation functions
    int evaluate(const Board& board) const;

    // Search functions
    int iterativeDeepening(Board& board, int maxDepth, bool maximizingPlayer);
    int search(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);
    int quiescenceSearch(Board& board, int alpha, int beta, bool maximizingPlayer) const;

    // Opening book and endgame tablebase
    Move getOpeningBookMove(const Board& board) const;
    Move getEndgameTablebaseMove(const Board& board) const;

    // Parameter tuning and backtesting
    void tuneParameters(const std::vector<int>& gameResults); // Placeholder for GameResult
    void backtestPieceSquareTables();

private:
    // Transposition table
    std::unordered_map<uint64_t, HashEntry> transpositionTable;

    // Zobrist hashing
    uint64_t zobristKeys[12][64]; // 12 piece types, 64 squares
    uint64_t zobristSide;         // Side to move

    // Piece-square tables
    PieceSquareTables pieceSquareTables;

    // Helper functions
    void initializeZobristKeys();
    uint64_t generateZobristHash(const Board& board, bool sideToMove) const;
    bool probeTranspositionTable(uint64_t hash, int depth, int alpha, int beta, int& score) const;
    void storeTransposition(uint64_t hash, int depth, int score, int flag);
    void moveOrdering(std::vector<Move>& moves, const Board& board, bool maximizingPlayer) const;

    // Evaluation components
    int evaluateMaterial(const Board& board) const;
    int evaluatePieceSquare(const Board& board) const;
    int evaluateKingSafety(const Board& board) const;
    int evaluatePassedPawns(const Board& board) const;
    int evaluateMobility(const Board& board, Color color) const;
    int evaluateThreats(const Board& board, Color color) const;

    // Backtesting helpers
    int simulateGames() const; // Simulate games and return a performance score
    void adjustPieceSquareTables(double factor);
};

#endif // EVALUATOR_H
