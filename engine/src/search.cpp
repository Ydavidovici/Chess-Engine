// src/search.cpp
#include "search.h"
#include <algorithm>

Search::Search(const Evaluator &evaluator, TranspositionTable &tt)
  : evaluator_(evaluator), tt_(tt)
{}

Move Search::findBestMove(Board &board, Color stm, int maxDepth) {
    Move best;
    for (int d = 1; d <= maxDepth; ++d) {
        int alpha = -INF, beta = +INF;
        Move localBest;
        auto moves = board.generateLegalMoves();
        if (moves.empty()) break;
        for (auto const &m : moves) {
            board.makeMove(m);
            Color opp = stm == Color::WHITE ? Color::BLACK : Color::WHITE;
            int score = -negamax(board, d-1, -beta, -alpha, opp);
            board.unmakeMove();
            if (score > alpha) {
                alpha = score;
                localBest = m;
            }
        }
        best = localBest;
    }
    return best;
}

int Search::negamax(Board &board, int depth, int alpha, int beta, Color stm) {
    uint64_t key = board.zobristKey();
    TranspositionTable::TTEntry ent;
    if (tt_.probe(key, ent) && ent.depth >= depth) {
        switch (ent.flag) {
          case TranspositionTable::EXACT:      return ent.value;
          case TranspositionTable::LOWERBOUND: alpha = std::max(alpha, ent.value); break;
          case TranspositionTable::UPPERBOUND: beta  = std::min(beta,  ent.value); break;
        }
        if (alpha >= beta) return ent.value;
    }

    if (depth == 0)
        return quiescence(board, alpha, beta, stm);

    auto moves = board.generateLegalMoves();
    if (moves.empty())
        return evaluator_.evaluateTerminal(board, stm);

    int origAlpha = alpha;
    Move bestChild;
    for (auto const &m : moves) {
        board.makeMove(m);
        Color opp = stm == Color::WHITE ? Color::BLACK : Color::WHITE;
        int score = -negamax(board, depth-1, -beta, -alpha, opp);
        board.unmakeMove();
        if (score >= beta) {
            tt_.store(key, score, depth, m, TranspositionTable::LOWERBOUND);
            return beta;
        }
        if (score > alpha) {
            alpha     = score;
            bestChild = m;
        }
    }

    auto flag =
      (alpha <= origAlpha) ? TranspositionTable::UPPERBOUND
    : (alpha >= beta)      ? TranspositionTable::LOWERBOUND
                            : TranspositionTable::EXACT;

    tt_.store(key, alpha, depth, bestChild, flag);
    return alpha;
}

int Search::quiescence(Board &board, int alpha, int beta, Color stm) {
    int stand = evaluator_.evaluate(board, stm);
    if (stand >= beta) return beta;
    alpha = std::max(alpha, stand);

    auto all = board.generateLegalMoves();
    std::vector<Move> caps;
    for (auto const &m : all) {
        if (m.isCapture() || m.type == MoveType::PROMOTION)
            caps.push_back(m);
    }
    for (auto const &m : caps) {
        board.makeMove(m);
        Color opp = stm == Color::WHITE ? Color::BLACK : Color::WHITE;
        int score = -quiescence(board, -beta, -alpha, opp);
        board.unmakeMove();
        if (score >= beta) return beta;
        alpha = std::max(alpha, score);
    }
    return alpha;
}
