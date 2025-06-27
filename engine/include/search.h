// include/search.h
#pragma once
#include "board.h"
#include "move.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include <limits>
#include <vector>

class Search {
public:
    static constexpr int INF = std::numeric_limits<int>::max() / 2;

    Search(const Evaluator &evaluator, TranspositionTable &tt);

    // returns best move for stm up to maxDepth plies
    Move findBestMove(Board &board, Color stm, int maxDepth);

private:
    const Evaluator &evaluator_;
    TranspositionTable &tt_;

    int negamax(Board &board, int depth, int alpha, int beta, Color stm);
    int quiescence(Board &board, int alpha, int beta, Color stm);
};
