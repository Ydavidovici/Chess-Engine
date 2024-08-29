#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "board.h"
#include <cstdint>

class Evaluator {
public:
    int evaluate(const Board& board) const;

private:
    // Declare countBits function here
    int countBits(uint64_t bitboard) const;
};

#endif // EVALUATOR_H
