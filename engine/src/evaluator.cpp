// evaluator.cpp
#include "evaluator.h"
#include <algorithm>
#include <iostream>

constexpr int Evaluator::pieceValues[12];

Evaluator::Evaluator() : maxQDepth(2) {
    // 1) build piece‑square tables
    initializePieceSquareTables();

    // 2) init Zobrist
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    zobristKeys.assign(12, std::vector<uint64_t>(64));
    for (int i = 0; i < 12; ++i)
        for (int sq = 0; sq < 64; ++sq)
            zobristKeys[i][sq] = dist(rng);
    zobristSide = dist(rng);
}

void Evaluator::initializePieceSquareTables() {
    // Pawn
    whitePawnTable = {
            0,   0,   0,   0,   0,   0,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            10,  10,  20,  30,  30,  20,  10,  10,
            5,   5,  10,  25,  25,  10,   5,   5,
            0,   0,   0,  20,  20,   0,   0,   0,
            5,  -5, -10,   0,   0, -10,  -5,   5,
            5,  10,  10, -20, -20,  10,  10,   5,
            0,   0,   0,   0,   0,   0,   0,   0
    };
    // Knight
    whiteKnightTable = {
            -50,-40,-30,-30,-30,-30,-40,-50,
            -40,-20,  0,  0,  0,  0,-20,-40,
            -30,  0, 10, 15, 15, 10,  0,-30,
            -30,  5, 15, 20, 20, 15,  5,-30,
            -30,  0, 15, 20, 20, 15,  0,-30,
            -30,  5, 10, 15, 15, 10,  5,-30,
            -40,-20,  0,  5,  5,  0,-20,-40,
            -50,-40,-30,-30,-30,-30,-40,-50
    };
    // Bishop
    whiteBishopTable = {
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -20,-10,-10,-10,-10,-10,-10,-20
    };
    // Rook
    whiteRookTable = {
            0,  0,  0,  0,  0,  0,  0,  0,
            5, 10, 10, 10, 10, 10, 10,  5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            0,  0,  0,  5,  5,  0,  0,  0
    };
    // Queen
    whiteQueenTable = {
            -20,-10,-10, -5, -5,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5,  5,  5,  5,  0,-10,
            -5,  0,  5,  5,  5,  5,  0, -5,
            0,  0,  5,  5,  5,  5,  0, -5,
            -10,  5,  5,  5,  5,  5,  0,-10,
            -10,  0,  5,  0,  0,  0,  0,-10,
            -20,-10,-10, -5, -5,-10,-10,-20
    };
    // King middle-game
    whiteKingTableMG = {
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            20, 20,  0,  0,  0,  0, 20, 20,
            20, 30, 10,  0,  0, 10, 30, 20
    };
    // King end‑game
    whiteKingTableEG = {
            -50,-40,-30,-20,-20,-30,-40,-50,
            -30,-20,-10,  0,  0,-10,-20,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-10, 30, 40, 40, 30,-10,-30,
            -30,-10, 30, 40, 40, 30,-10,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-30,  0,  0,  0,  0,-30,-30,
            -50,-30,-30,-30,-30,-30,-30,-50
    };

    // mirror for Black
    blackPawnTable   = { whitePawnTable.rbegin(),   whitePawnTable.rend() };
    blackKnightTable = { whiteKnightTable.rbegin(), whiteKnightTable.rend() };
    blackBishopTable = { whiteBishopTable.rbegin(), whiteBishopTable.rend() };
    blackRookTable   = { whiteRookTable.rbegin(),   whiteRookTable.rend() };
    blackQueenTable  = { whiteQueenTable.rbegin(),  whiteQueenTable.rend() };
    blackKingTableMG = { whiteKingTableMG.rbegin(), whiteKingTableMG.rend() };
    blackKingTableEG = { whiteKingTableEG.rbegin(), whiteKingTableEG.rend() };
}

uint64_t Evaluator::generateZobristHash(const Board& board,
                                        bool sideToMove) const {
    uint64_t h = 0, bit;
    for (int sq = 0; sq < 64; ++sq) {
        bit = (1ULL << sq);
        if (board.getWhitePawns()   & bit) h ^= zobristKeys[0][sq];
        if (board.getWhiteKnights() & bit) h ^= zobristKeys[1][sq];
        if (board.getWhiteBishops() & bit) h ^= zobristKeys[2][sq];
        if (board.getWhiteRooks()   & bit) h ^= zobristKeys[3][sq];
        if (board.getWhiteQueens()  & bit) h ^= zobristKeys[4][sq];
        if (board.getWhiteKings()   & bit) h ^= zobristKeys[5][sq];
        if (board.getBlackPawns()   & bit) h ^= zobristKeys[6][sq];
        if (board.getBlackKnights() & bit) h ^= zobristKeys[7][sq];
        if (board.getBlackBishops() & bit) h ^= zobristKeys[8][sq];
        if (board.getBlackRooks()   & bit) h ^= zobristKeys[9][sq];
        if (board.getBlackQueens()  & bit) h ^= zobristKeys[10][sq];
        if (board.getBlackKings()   & bit) h ^= zobristKeys[11][sq];
    }
    if (sideToMove) h ^= zobristSide;
    return h;
}

int Evaluator::evaluateMaterial(const Board& board) const {
    auto pop = [](uint64_t b){ return __builtin_popcountll(b); };
    int m = 0;
    m += 100 * pop(board.getWhitePawns());
    m += 320 * pop(board.getWhiteKnights());
    m += 330 * pop(board.getWhiteBishops());
    m += 500 * pop(board.getWhiteRooks());
    m += 900 * pop(board.getWhiteQueens());
    m += 20000 * pop(board.getWhiteKings());
    m -= 100 * pop(board.getBlackPawns());
    m -= 320 * pop(board.getBlackKnights());
    m -= 330 * pop(board.getBlackBishops());
    m -= 500 * pop(board.getBlackRooks());
    m -= 900 * pop(board.getBlackQueens());
    m -= 20000 * pop(board.getBlackKings());
    return m;
}

int Evaluator::evaluatePositional(const Board& board) const {
    int score = 0;
    auto scan = [&](uint64_t bb, const std::vector<int>& tbl, int sign){
        while (bb) {
            int sq = __builtin_ctzll(bb);
            score += sign * tbl[sq];
            bb &= bb - 1;
        }
    };
    scan(board.getWhitePawns(),   whitePawnTable,   +1);
    scan(board.getWhiteKnights(), whiteKnightTable, +1);
    scan(board.getWhiteBishops(), whiteBishopTable, +1);
    scan(board.getWhiteRooks(),   whiteRookTable,   +1);
    scan(board.getWhiteQueens(),  whiteQueenTable,  +1);
    scan(board.getWhiteKings(),   whiteKingTableMG, +1);

    scan(board.getBlackPawns(),   blackPawnTable,   -1);
    scan(board.getBlackKnights(), blackKnightTable, -1);
    scan(board.getBlackBishops(), blackBishopTable, -1);
    scan(board.getBlackRooks(),   blackRookTable,   -1);
    scan(board.getBlackQueens(),  blackQueenTable,  -1);
    scan(board.getBlackKings(),   blackKingTableMG, -1);
    return score;
}

int Evaluator::evaluate(const Board& board) const {
    return evaluateMaterial(board) + evaluatePositional(board);
}

std::vector<Move> Evaluator::orderMoves(std::vector<Move> moves,
                                        const Board& /*board*/,
                                        bool /*maximizingPlayer*/) const {
    std::stable_sort(moves.begin(), moves.end(),
                     [](auto &a, auto &b){
                         return a.isCapture() && !b.isCapture();
                     });
    return moves;
}

int Evaluator::quiescenceSearch(Board board,
                                int alpha, int beta,
                                bool maximizingPlayer,
                                int depth) {
    if (depth >= maxQDepth)
        return evaluate(board);

    int standPat = evaluate(board);
    if (maximizingPlayer) {
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);
        auto moves = board.generateLegalMoves(Color::WHITE);
        for (auto &m : moves) {
            if (!m.isCapture()) continue;
            Board nb = board; nb.makeMove(m, Color::WHITE);
            int score = -quiescenceSearch(nb, -beta, -alpha, false, depth+1);
            if (score >= beta) return beta;
            alpha = std::max(alpha, score);
        }
        return alpha;
    } else {
        if (standPat <= alpha) return alpha;
        beta = std::min(beta, standPat);
        auto moves = board.generateLegalMoves(Color::BLACK);
        for (auto &m : moves) {
            if (!m.isCapture()) continue;
            Board nb = board; nb.makeMove(m, Color::BLACK);
            int score = -quiescenceSearch(nb, -beta, -alpha, true, depth+1);
            if (score <= alpha) return alpha;
            beta = std::min(beta, score);
        }
        return beta;
    }
}

std::pair<int, Move> Evaluator::alphaBeta(Board board,
                                          int depth,
                                          int alpha,
                                          int beta,
                                          bool maximizingPlayer) {
    int alphaOrig = alpha;
    Move bestMove;
    int bestScore = maximizingPlayer
                    ? std::numeric_limits<int>::min()
                    : std::numeric_limits<int>::max();

    uint64_t h = generateZobristHash(board, maximizingPlayer);
    if (auto it = transpositionTable.find(h); it != transpositionTable.end()) {
        auto &e = it->second;
        if (e.depth >= depth) {
            if (e.flag == EXACT)      return {e.score, {}};
            if (e.flag == ALPHA_FLAG && e.score <= alpha) return {alpha, {}};
            if (e.flag == BETA_FLAG  && e.score >= beta)  return {beta,  {}};
        }
    }

    if (depth == 0) {
        return { quiescenceSearch(board, alpha, beta,
                                  maximizingPlayer, 0), {} };
    }

    auto moves = board.generateLegalMoves(
            maximizingPlayer ? Color::WHITE : Color::BLACK
    );
    moves = orderMoves(moves, board, maximizingPlayer);
    if (moves.empty()) {
        return maximizingPlayer
               ? std::make_pair(-100000, Move())
               : std::make_pair( 100000, Move());
    }

    for (auto &m : moves) {
        Board nb = board;
        nb.makeMove(m, maximizingPlayer ? Color::WHITE : Color::BLACK);
        int score = -alphaBeta(nb, depth-1, -beta, -alpha,
                               !maximizingPlayer).first;
        if ((maximizingPlayer && score > bestScore) ||
            (!maximizingPlayer && score < bestScore)) {
            bestScore = score;
            bestMove  = m;
        }
        if (maximizingPlayer) alpha = std::max(alpha, score);
        else                beta  = std::min(beta,  score);
        if (alpha >= beta) break;
    }

    TTEntry e{ depth, bestScore,
               bestScore <= alphaOrig ? BETA_FLAG :
               bestScore >= beta      ? ALPHA_FLAG :
               EXACT };
    transpositionTable[h] = e;
    return {bestScore, bestMove};
}

Move Evaluator::iterativeDeepening(const Board& board,
                                   int maxDepth,
                                   bool maximizingPlayer) {
    Move bestMove;
    for (int d = 1; d <= maxDepth; ++d) {
        auto [score, mv] = alphaBeta(board, d,
                                     std::numeric_limits<int>::min(),
                                     std::numeric_limits<int>::max(),
                                     maximizingPlayer);
        if (mv.isValid()) bestMove = mv;
        std::cout << "Depth " << d
                  << " completed: Best score " << score
                  << ", Best move: " << bestMove.toString()
                  << "\n";
    }
    return bestMove;
}