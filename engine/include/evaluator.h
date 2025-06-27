// include/evaluator.h
#pragma once

#include "board.h"
#include "move.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <random>
#include <limits>
#include <utility>

class Evaluator {
public:
    /**
     * @param maxQDepth  maximum recursive depth for quiescence before doing a static evaluate()
     */
    explicit Evaluator(unsigned maxQDepth = 2);

    /**
     * Full evaluation: returns +score if White is better to move, –score if Black.
     * Handles checkmates (±∞ → ±100000), draws (0), otherwise material+positional.
     */
    int evaluate(const Board &board, Color stm) const;

    /**
     * Leaf-only evaluation used by Search::negamax when depth==0.
     * Same terminal handling, otherwise simply calls evaluate().
     */
    int evaluateTerminal(const Board &board, Color stm) const;

    // (Optional) Expose these for direct use:
    int quiescenceSearch(Board board,
                         int alpha, int beta,
                         bool maximizingPlayer,
                         int depth);
    std::pair<int,Move> alphaBeta(Board board,
                                  int depth,
                                  int alpha, int beta,
                                  bool maximizingPlayer);
    Move iterativeDeepening(const Board &board,
                            int maxDepth,
                            bool maximizingPlayer);

private:
    void initializePieceSquareTables();
    uint64_t generateZobristHash(const Board &board,
                                 bool sideToMove) const;

    int evaluateMaterial(const Board &board) const;
    int evaluatePositional(const Board &board) const;

    unsigned maxQDepth;

    // Piece‐values and piece‐square tables
    static constexpr int PST_COUNT = Board::PieceTypeCount;
    static constexpr int pieceValues[PST_COUNT]
        = { 100, 320, 330, 500, 900, 20000 };

    std::vector<int> whitePawnTable,   whiteKnightTable,
                     whiteBishopTable, whiteRookTable,
                     whiteQueenTable,  whiteKingTableMG,
                     whiteKingTableEG;
    std::vector<int> blackPawnTable,   blackKnightTable,
                     blackBishopTable, blackRookTable,
                     blackQueenTable,  blackKingTableMG,
                     blackKingTableEG;

    // Zobrist
    std::vector<std::vector<uint64_t>> zobristKeys;  // [12][64]
    uint64_t zobristSide;

    // Tiny TT for alphaBeta
    struct TTEntry { int depth, score, flag; };
    enum : int { EXACT = 0, ALPHA_FLAG = 1, BETA_FLAG = 2 };
    std::unordered_map<uint64_t,TTEntry> transpositionTable;

    // Fallback zero‐initialized PST
    static const int pst[PST_COUNT][64];

    // Move ordering helper
    static std::vector<Move> orderMoves(std::vector<Move> moves,
                                        bool maximizingPlayer);
};
