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

private:
    const Evaluator& evaluator_;
    TranspositionTable& tt_;
    TimeManager tm_;

    int negamax(Board& board, int depth, int alpha, int beta, int plyFromRoot);
    int quiescence(Board& board, int alpha, int beta);
    void orderMoves(std::vector<Move>& moves);
};