// include/search.h
#pragma once

#include "board.h"
#include "move.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "timeman.h"
#include <limits>
#include <vector>
#include <cstdint>

class Search {
public:
    static constexpr int INF = std::numeric_limits<int>::max() / 2;

    Search(const Evaluator &evaluator, TranspositionTable &tt);

    /**
     * Legacy overload: no time management (infinite time budget).
     */
    Move findBestMove(Board &board,
                      Color stm,
                      int maxDepth);

    /**
     * Timed search up to maxDepth plies, subject to given TimeManager.
     */
    Move findBestMove(Board &board,
                      Color stm,
                      int maxDepth,
                      TimeManager &tm);

private:
    const Evaluator &evaluator_;
    TranspositionTable &tt_;

    int negamax(Board &board, int depth, int alpha, int beta, Color stm, TimeManager &tm);
    int quiescence(Board &board, int alpha, int beta, Color stm, TimeManager &tm);
};