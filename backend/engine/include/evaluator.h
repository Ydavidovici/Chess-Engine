#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "board.h"
#include "move.h"

// Piece-square table structure
class PieceSquareTables {
public:
    int pawn_table[64];
    int knight_table[64];
    int bishop_table[64];
    int rook_table[64];
    int queen_table[64];
    int king_table_mg[64];  // Middlegame King table
    int king_table_eg[64];  // Endgame King table

    int pawn_table_black[64];
    int knight_table_black[64];
    int bishop_table_black[64];
    int rook_table_black[64];
    int queen_table_black[64];
    int king_table_black_mg[64];
    int king_table_black_eg[64];

    void initialize();  // Function to initialize piece-square tables
};

// Transposition table entry structure
struct HashEntry {
    uint64_t hash;   // Zobrist hash of the position
    int depth;       // Search depth
    int score;       // Evaluation score
    int flag;        // Exact, alpha, or beta flag
    int type;        // Node type (PV, all-node, etc.)
};

// Evaluator class
class Evaluator {
private:
    PieceSquareTables pieceSquareTables;

    // Helper functions
    int countBits(uint64_t bitboard) const;
    int taperedEval(const Board& board, int phase) const;
    int evaluateMaterial(const Board& board) const;
    int evaluatePieceSquare(const Board& board) const;
    int evaluateKingSafety(const Board& board) const;
    int evaluatePassedPawns(const Board& board) const;
    int evaluatePawnStructure(const Board& board) const;
    int evaluateBishopPair(const Board& board) const;
    int evaluateRookOpenFile(const Board& board) const;
    int evaluateMobility(const Board& board, Color color) const;
    int evaluateThreats(const Board& board, Color color) const;
    int quiescenceSearch(Board& board, int alpha, int beta, bool maximizingPlayer) const;
    int search(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable);
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable);
    int nullMovePruning(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable);
    void moveOrdering(std::vector<Move>& moves, const Board& board, bool maximizingPlayer) const;
    void storeTransposition(std::unordered_map<uint64_t, HashEntry>& transpositionTable, uint64_t hash, int score, int depth, int flag, int type);

    // Game phase calculation
    int calculateGamePhase(const Board& board) const;

    // Functions for tuning and backtesting
    void adjustPieceSquareTables(double factor);
    void backtestPieceSquareTables();
    int simulateGames();

public:
    Evaluator();  // Constructor
    int evaluate(const Board& board) const;  // Full evaluation function
    int iterativeDeepening(Board& board, int maxDepth, bool maximizingPlayer);

    // Stockfish integration
    void integrateStockfish(const Board& board) const;

    // Opening book and endgame tablebase support
    Move getOpeningBookMove(const Board& board) const;
    Move getEndgameTablebaseMove(const Board& board) const;

    // Parameter tuning
    void tuneParameters(const GameResult& result);

    // Legal move generation (assuming the Board and Move classes exist)
    std::vector<Move> generateLegalMoves(const Board& board, Color color) const;
};

#endif // EVALUATOR_H
