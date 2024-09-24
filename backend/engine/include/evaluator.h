#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "board.h"
#include <cstdint>
#include <unordered_map>

// Structure for hash table entry
struct HashEntry {
    uint64_t key;
    int depth;
    int score;
    int flag;
};

class Evaluator {
public:
    Evaluator();

    int evaluate(const Board& board) const;
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable) const;
    std::vector<Move> generateLegalMoves(const Board& board, Color color) const;

private:
    // Helper functions
    int countBits(uint64_t bitboard) const;
};

#endif // EVALUATOR_H
