// evaluator.h
#pragma once

#include "board.h"
#include "move.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <utility>

enum TTFlag { EXACT = 0, ALPHA_FLAG = 1, BETA_FLAG = 2 };

struct TTEntry {
    int depth;
    int score;
    TTFlag flag;
};

class Evaluator {
public:
    Evaluator();

    // Full static evaluation
    int evaluate(const Board& board) const;

    // Iterative deepening: returns best move found up to maxDepth
    Move iterativeDeepening(const Board& board, int maxDepth, bool maximizingPlayer);

private:
    // Build PSTs
    void initializePieceSquareTables();

    // Zobrist
    uint64_t generateZobristHash(const Board& board, bool sideToMove) const;

    // Material & positional
    int evaluateMaterial(const Board& board) const;
    int evaluatePositional(const Board& board) const;

    // Quiescence search
    int quiescenceSearch(Board board, int alpha, int beta,
                         bool maximizingPlayer, int depth);

    // Move ordering
    std::vector<Move> orderMoves(const std::vector<Move>& moves,
                                 const Board& board,
                                 bool maximizingPlayer) const;

    // Alpha‑beta recursive
    std::pair<int,Move> alphaBeta(Board board, int depth,
                                  int alpha, int beta,
                                  bool maximizingPlayer);

    // Piece values for MVV‑LVA and material
    static constexpr int pieceValues[12] = {
            100, 320, 330, 500,  900, 20000,
            -100,-320,-330,-500, -900,-20000
    };

    // Configuration
    int maxQDepth;

    // Zobrist keys
    std::vector<std::vector<uint64_t>> zobristKeys;
    uint64_t zobristSide;

    // Transposition table
    std::unordered_map<uint64_t,TTEntry> transpositionTable;

    // Piece‑square tables
    std::vector<int> whitePawnTable,
            whiteKnightTable,
            whiteBishopTable,
            whiteRookTable,
            whiteQueenTable,
            whiteKingTableMG,
            whiteKingTableEG;

    std::vector<int> blackPawnTable,
            blackKnightTable,
            blackBishopTable,
            blackRookTable,
            blackQueenTable,
            blackKingTableMG,
            blackKingTableEG;
};