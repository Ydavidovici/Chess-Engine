#pragma once

#include "board.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "timeManager.h"
#include "move.h"
#include <vector>

class Search {
public:
    Search(const Evaluator& evaluator, TranspositionTable& tt);

    Move findBestMove(Board& board, int maxDepth, int timeLeftMs = 0, int incrementMs = 0);

    struct SearchStats {
        long long totalNodes = 0;
        long long qNodes = 0;
        long long ttHits = 0;
        long long betaCutoffs = 0;
        long long firstMoveCutoffs = 0;

        void operator+=(const SearchStats& other) {
            totalNodes += other.totalNodes;
            qNodes += other.qNodes;
            ttHits += other.ttHits;
            betaCutoffs += other.betaCutoffs;
            firstMoveCutoffs += other.firstMoveCutoffs;
        }

        void reset() {
            totalNodes = 0;
            qNodes = 0;
            ttHits = 0;
            betaCutoffs = 0;
            firstMoveCutoffs = 0;
        }
    };

    const SearchStats& getStats() const { return stats_; }
    void resetStats() { stats_.reset(); }

    uint64_t getNodes() const { return stats_.totalNodes; }

private:
    const Evaluator& evaluator_;
    TranspositionTable& tt_;
    TimeManager tm_;
    SearchStats stats_;
    int history_[2][64][64];

    int negamax(Board& board, int depth, int alpha, int beta, int plyFromRoot);
    int quiescence(Board& board, int alpha, int beta, int plyFromRoot);
    void orderMoves(Board& board, std::vector<Move>& moves);
};