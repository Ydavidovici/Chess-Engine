// backend/engine/include/evaluator.h

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "board.h"

class Evaluator {
public:
    int evaluate(const Board& board) const;
};

#endif // EVALUATOR_H
