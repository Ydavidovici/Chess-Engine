// src/search.cpp

#include "search.h"
#include <algorithm>
#include <limits>
#include <iostream>

static constexpr int INF = std::numeric_limits<int>::max();

Search::Search(const Evaluator &evaluator, TranspositionTable &tt)
  : evaluator_(evaluator), tt_(tt)
{}

Move Search::findBestMove(Board &board,
                          Color stm,
                          int maxDepth,
                          TimeManager &tm)
{
    Color opp = (stm == Color::WHITE ? Color::BLACK : Color::WHITE);

    // 1) Gather all root moves and pick the first as a safe default
    auto rootMoves = board.generateLegalMoves();
    Move best = rootMoves.front();
    std::cerr << "[search] rootLegal=(" << rootMoves.size()
              << ") defaulting best=" << best.toString() << "\n";

    std::cerr << "[search] ==== START ITERATIVE DEEPENING (maxDepth="
              << maxDepth << ") ====\n";

    // 2) Iterative deepening up to maxDepth
    for (int depth = 1; depth <= maxDepth; ++depth) {
        std::cerr << "[search] -- depth=" << depth << "\n";
        if (tm.isTimeUp()) {
            std::cerr << "[search]    time's up before depth " << depth
                      << ", returning best so far=" << best.toString() << "\n";
            break;
        }

        int alpha = -INF, beta = +INF;
        // start localBest as the previous best so it's always valid
        Move localBest = best;

        auto moves = board.generateLegalMoves();
        std::cerr << "[search]   moves to try: " << moves.size() << "\n";

        for (auto const &m : moves) {
            if (tm.isTimeUp()) {
                std::cerr << "[search]      time up mid-move-loop\n";
                break;
            }

            std::cerr << "[search]    eval move " << m.toString() << "\n";
            board.makeMove(m);
            std::cerr << "[search]      calling negamax for "
                      << m.toString() << "\n";

            int score = -negamax(board,
                                 depth - 1,
                                 -beta,
                                 -alpha,
                                 opp,
                                 tm);
            board.unmakeMove();

            std::cerr << "[search]    move " << m.toString()
                      << " → score=" << score
                      << "  (prev α=" << alpha << ")\n";

            if (score > alpha) {
                alpha     = score;
                localBest = m;
                std::cerr << "[search]      ^ new best at depth "
                          << depth << ": " << localBest.toString()
                          << " (α=" << alpha << ")\n";
            }
        }

        // update best to whatever we found at this depth
        best = localBest;
    }

    std::cerr << "[search] ==== DONE: best=" << best.toString()
              << " ====\n";
    return best;
}

int Search::negamax(Board &board,
                    int depth,
                    int alpha,
                    int beta,
                    Color stm,
                    TimeManager &tm)
{
    if (tm.isTimeUp())
        return alpha;

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
        return quiescence(board, alpha, beta, stm, tm);

    auto moves = board.generateLegalMoves();
    if (moves.empty())
        return evaluator_.evaluateTerminal(board, stm);

    int origAlpha = alpha;
    Move bestChild = moves.front();

    for (auto const &m : moves) {
        if (tm.isTimeUp()) break;
        board.makeMove(m);
        Color opp = (stm == Color::WHITE ? Color::BLACK : Color::WHITE);
        int score = -negamax(board,
                             depth - 1,
                             -beta,
                             -alpha,
                             opp,
                             tm);
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

    auto flag = (alpha <= origAlpha)
        ? TranspositionTable::UPPERBOUND
        : (alpha >= beta)
          ? TranspositionTable::LOWERBOUND
          : TranspositionTable::EXACT;
    tt_.store(key, alpha, depth, bestChild, flag);
    return alpha;
}

int Search::quiescence(Board &board,
                       int alpha,
                       int beta,
                       Color stm,
                       TimeManager &tm)
{
    if (tm.isTimeUp())
        return alpha;

    int stand = evaluator_.evaluate(board, stm);
    if (stand >= beta) return beta;
    alpha = std::max(alpha, stand);

    auto allMoves = board.generateLegalMoves();
    std::vector<Move> caps;
    for (auto const &m : allMoves) {
        if (m.isCapture() || m.type == MoveType::PROMOTION)
            caps.push_back(m);
    }

    for (auto const &m : caps) {
        if (tm.isTimeUp()) break;
        board.makeMove(m);
        Color opp = (stm == Color::WHITE ? Color::BLACK : Color::WHITE);
        int score = -quiescence(board, -beta, -alpha, opp, tm);
        board.unmakeMove();

        if (score >= beta) return beta;
        alpha = std::max(alpha, score);
    }
    return alpha;
}
