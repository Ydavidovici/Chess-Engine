#include "search.h"
#include <algorithm>
#include <iostream>
#include <limits>

static constexpr int INF = 1000000;
static constexpr int MATE_SCORE = 100000;

Search::Search(const Evaluator& evaluator, TranspositionTable& tt)
    : evaluator_(evaluator), tt_(tt) {}

Move Search::findBestMove(Board& board, int maxDepth, int timeLeftMs, int incrementMs) {
    nodes_ = 0;

    if (timeLeftMs > 0) {
        tm_.start(timeLeftMs, incrementMs, 0);
    } else {
        tm_.start(2000, 0, 0);
    }
    Move bestMove;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (tm_.isTimeUp()) break;

        int alpha = -INF;
        int beta = INF;

        auto rootMoves = board.generateLegalMoves();
        if (rootMoves.empty()) break;

        orderMoves(rootMoves);

        Move currentBestMove = rootMoves[0];
        int currentBestScore = -INF;

        for (const auto& move : rootMoves) {
            board.makeMove(move);

            int score = -negamax(board, depth - 1, -beta, -alpha, 1);

            board.unmakeMove();

            if (tm_.isTimeUp()) break;

            if (score > currentBestScore) {
                currentBestScore = score;
                currentBestMove = move;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        if (!tm_.isTimeUp()) {
            bestMove = currentBestMove;
            std::cout << "info depth " << depth << " score cp " << currentBestScore << " pv " << bestMove.toString() << "\n";
        }
    }

    return bestMove;
}

int Search::negamax(Board& board, int depth, int alpha, int beta, int plyFromRoot) {
    nodes_++;

    if ((plyFromRoot % 2048) == 0 && tm_.isTimeUp()) return 0;

    uint64_t key = evaluator_.generateZobristHash(board, board.sideToMove() == Color::WHITE);

    TranspositionTable::TTEntry ent;
    if (tt_.probe(key, ent) && ent.depth >= depth) {
        if (ent.flag == TranspositionTable::EXACT) return ent.value;
        if (ent.flag == TranspositionTable::LOWERBOUND) alpha = std::max(alpha, ent.value);
        if (ent.flag == TranspositionTable::UPPERBOUND) beta = std::min(beta, ent.value);
        if (alpha >= beta) return ent.value;
    }

    if (depth == 0) {
        return quiescence(board, alpha, beta);
    }

    auto moves = board.generateLegalMoves();
    if (moves.empty()) {
        int score = evaluator_.evaluateTerminal(board, board.sideToMove());
        if (score == -MATE_SCORE) score += plyFromRoot;
        return score;
    }

    orderMoves(moves);

    int bestScore = -INF;
    Move bestMoveInNode;

    for (const auto& move : moves) {
        board.makeMove(move);
        int score = -negamax(board, depth - 1, -beta, -alpha, plyFromRoot + 1);
        board.unmakeMove();

        if (tm_.isTimeUp()) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMoveInNode = move;
        }
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            tt_.store(key, beta, depth, move, TranspositionTable::LOWERBOUND);
            return beta;
        }
    }

    int flag = TranspositionTable::EXACT;
    if (bestScore <= alpha) flag = TranspositionTable::UPPERBOUND;
    else if (bestScore >= beta) flag = TranspositionTable::LOWERBOUND;

    tt_.store(key, bestScore, depth, bestMoveInNode, flag);
    return bestScore;
}

int Search::quiescence(Board& board, int alpha, int beta) {
    nodes_++;

    int standPat = evaluator_.evaluate(board, board.sideToMove());
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    auto allMoves = board.generateLegalMoves();
    std::vector<Move> captures;
    captures.reserve(allMoves.size());
    for (const auto& m : allMoves) {
        if (m.isCapture() || m.type == MoveType::PROMOTION) {
            captures.push_back(m);
        }
    }

    orderMoves(captures);

    for (const auto& move : captures) {
        board.makeMove(move);
        int score = -quiescence(board, -beta, -alpha);
        board.unmakeMove();

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

void Search::orderMoves(std::vector<Move>& moves) {
    std::stable_sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        bool aIsCap = a.isCapture();
        bool bIsCap = b.isCapture();
        bool aIsPromo = (a.type == MoveType::PROMOTION);
        bool bIsPromo = (b.type == MoveType::PROMOTION);

        if (aIsCap != bIsCap) return aIsCap > bIsCap;

        if (aIsPromo != bIsPromo) return aIsPromo > bIsPromo;

        return false;
    });
}
