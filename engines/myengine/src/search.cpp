#include "search.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <cstring>

static constexpr int INF = 1000000;
static constexpr int MATE_SCORE = 100000;

static int getMvvLvaScore(const Board& board, const Move& move) {
    if (!move.isCapture()) return 0;

    Board::PieceIndex victim = board.getPieceAt(move.end);
    Board::PieceIndex attacker = board.getPieceAt(move.start);

    if (victim == Board::PieceTypeCount) {
        if (move.type == MoveType::EN_PASSANT) victim = Board::PAWN;
        else return 0;
    }

    int victimScore = 0;
    switch(victim) {
    case Board::PAWN:   victimScore = 100; break;
    case Board::KNIGHT: victimScore = 200; break;
    case Board::BISHOP: victimScore = 300; break;
    case Board::ROOK:   victimScore = 400; break;
    case Board::QUEEN:  victimScore = 500; break;
    case Board::KING:   victimScore = 600; break;
    default: break;
    }

    int attackerScore = 0;
    switch(attacker) {
    case Board::PAWN:   attackerScore = 1; break;
    case Board::KNIGHT: attackerScore = 2; break;
    case Board::BISHOP: attackerScore = 3; break;
    case Board::ROOK:   attackerScore = 4; break;
    case Board::QUEEN:  attackerScore = 5; break;
    case Board::KING:   attackerScore = 6; break;
    default: break;
    }

    return victimScore - attackerScore;
}

Search::Search(const Evaluator& evaluator, TranspositionTable& tt)
    : evaluator_(evaluator), tt_(tt) {}

Move Search::findBestMove(Board& board, int maxDepth, int timeLeftMs, int incrementMs) {
    stats_.reset();

    std::memset(history_, 0, sizeof history_);

    if (timeLeftMs > 0) {
        tm_.start(timeLeftMs, incrementMs, 0);
    } else {
        tm_.start(50000, 0, 0);
    }

    Move bestMove;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (tm_.isTimeUp()) break;

        int alpha = -INF;
        int beta = INF;

        auto rootMoves = board.generateLegalMoves();
        if (rootMoves.empty()) break;

        orderMoves(board, rootMoves);

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
    stats_.totalNodes++;

    if ((plyFromRoot % 2048) == 0 && tm_.isTimeUp()) return 0;

    if (plyFromRoot > 0 && (board.isThreefoldRepetition() || board.isFiftyMoveDraw())) {
        return 0;
    }

    uint64_t key = board.zobristKey();

    TranspositionTable::TTEntry ent;
    if (tt_.probe(key, ent) && ent.depth >= depth) {
        stats_.ttHits++;
        if (ent.flag == TranspositionTable::EXACT) return ent.value;
        if (ent.flag == TranspositionTable::LOWERBOUND) alpha = std::max(alpha, ent.value);
        if (ent.flag == TranspositionTable::UPPERBOUND) beta = std::min(beta, ent.value);
        if (alpha >= beta) {
            stats_.betaCutoffs++;
            return ent.value;
        }
    }

    if (depth == 0) {
        return quiescence(board, alpha, beta, plyFromRoot);
    }

    auto moves = board.generateLegalMoves();
    if (moves.empty()) {
        int score = 0;
        if (board.inCheck(board.sideToMove())) {
            score = -MATE_SCORE + plyFromRoot;
        } else {
            score = 0;
        }
        return score;
    }

    orderMoves(board, moves);

    int bestScore = -INF;
    Move bestMoveInNode;
    int movesSearched = 0;

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
            stats_.betaCutoffs++;
            if (movesSearched == 0) stats_.firstMoveCutoffs++;

            if (!move.isCapture()) {
                int side = static_cast<int>(board.sideToMove());
                history_[side][move.start][move.end] += depth * depth;

                if (history_[side][move.start][move.end] > 10000000) {
                    history_[side][move.start][move.end] /= 2;
                }
            }

            tt_.store(key, beta, depth, move, TranspositionTable::LOWERBOUND);
            return beta;
        }
        movesSearched++;
    }

    int flag = TranspositionTable::EXACT;
    if (bestScore <= alpha) flag = TranspositionTable::UPPERBOUND;
    else if (bestScore >= beta) flag = TranspositionTable::LOWERBOUND;

    tt_.store(key, bestScore, depth, bestMoveInNode, flag);
    return bestScore;
}

int Search::quiescence(Board& board, int alpha, int beta, int plyFromRoot) {
    stats_.totalNodes++;
    stats_.qNodes++;

    if (plyFromRoot > 64) {
        return evaluator_.evaluate(board, board.sideToMove());
    }

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

    orderMoves(board, captures);

    for (const auto& move : captures) {
        board.makeMove(move);
        int score = -quiescence(board, -beta, -alpha, plyFromRoot + 1);
        board.unmakeMove();

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

void Search::orderMoves(Board& board, std::vector<Move>& moves) {
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        int scoreA = 0;
        int scoreB = 0;

        if (a.isCapture()) scoreA = getMvvLvaScore(board, a) + 100000;
        if (b.isCapture()) scoreB = getMvvLvaScore(board, b) + 100000;

        if (a.type == MoveType::PROMOTION) scoreA += 90000;
        if (b.type == MoveType::PROMOTION) scoreB += 90000;

        if (!a.isCapture()) {
            scoreA += history_[static_cast<int>(board.sideToMove())][a.start][a.end];
        }
        if (!b.isCapture()) {
            scoreB += history_[static_cast<int>(board.sideToMove())][b.start][b.end];
        }

        return scoreA > scoreB;
    });
}