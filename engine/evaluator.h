// evaluator.h
#pragma once

#include "board.h"
#include "move.h"
#include "types.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <utility>

/// Entry for the transposition table
struct TTEntry {
    int depth;
    int score;
    int flag;  // EXACT, ALPHA_FLAG, or BETA_FLAG
};

class Evaluator {
public:
    Evaluator();

    /// Full static evaluation: material + position
    int evaluate(const Board& board) const;

    /// Iterative deepening: returns the best move found up to maxDepth
    Move iterativeDeepening(const Board& board,
                            int maxDepth,
                            bool maximizingPlayer);

private:
    // Build piece‑square tables
    void initializePieceSquareTables();

    // Zobrist hashing
    uint64_t generateZobristHash(const Board& board,
                                 bool sideToMove) const;

    // Material & positional
    int evaluateMaterial(const Board& board) const;
    int evaluatePositional(const Board& board) const;

    // Quiescence search
    int quiescenceSearch(Board board,
                         int alpha, int beta,
                         bool maximizingPlayer,
                         int depth);

    // Move ordering (captures first)
    std::vector<Move> orderMoves(std::vector<Move> moves,
                                 const Board& board,
                                 bool maximizingPlayer) const;

    // Alpha‑beta search
    std::pair<int, Move> alphaBeta(Board board,
                                   int depth,
                                   int alpha, int beta,
                                   bool maximizingPlayer);

    // static piece‐values for MVV‑LVA & material
    static constexpr int pieceValues[12] = {
            100, 320, 330, 500,  900, 20000,
            -100,-320,-330,-500, -900,-20000
    };

    int maxQDepth;  // quiescence depth limit

    // Zobrist keys
    std::vector<std::vector<uint64_t>> zobristKeys;
    uint64_t zobristSide;

    // Transposition table
    std::unordered_map<uint64_t, TTEntry> transpositionTable;

    // Piece‑square tables
    std::vector<int> whitePawnTable,   whiteKnightTable,
            whiteBishopTable, whiteRookTable,
            whiteQueenTable,  whiteKingTableMG,
            whiteKingTableEG;

    std::vector<int> blackPawnTable,   blackKnightTable,
            blackBishopTable, blackRookTable,
            blackQueenTable,  blackKingTableMG,
            blackKingTableEG;
};