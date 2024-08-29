#include "evaluator.h"
#include "board.h"
#include <bitset>  // Include to use std::bitset

// Piece values for evaluation
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;

// Evaluate the board state
int Evaluator::evaluate(const Board& board) const {
    int score = 0;

    // Evaluate material count using bitboards
    score += PAWN_VALUE * countBits(board.getWhitePawns());
    score += KNIGHT_VALUE * countBits(board.getWhiteKnights());
    score += BISHOP_VALUE * countBits(board.getWhiteBishops());
    score += ROOK_VALUE * countBits(board.getWhiteRooks());
    score += QUEEN_VALUE * countBits(board.getWhiteQueens());
    score += 20000 * countBits(board.getWhiteKings());  // High value for King to avoid capture

    score -= PAWN_VALUE * countBits(board.getBlackPawns());
    score -= KNIGHT_VALUE * countBits(board.getBlackKnights());
    score -= BISHOP_VALUE * countBits(board.getBlackBishops());
    score -= ROOK_VALUE * countBits(board.getBlackRooks());
    score -= QUEEN_VALUE * countBits(board.getBlackQueens());
    score -= 20000 * countBits(board.getBlackKings());  // High value for King to avoid capture

    // Additional evaluation criteria can be added here (e.g., piece positioning, king safety)

    return score;
}

// Count the number of bits set to 1 in a bitboard (Hamming weight)
int Evaluator::countBits(uint64_t bitboard) const {
    return std::bitset<64>(bitboard).count();
}
