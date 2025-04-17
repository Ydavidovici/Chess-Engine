// evaluator.cpp
#include "evaluator.h"
#include <random>
#include <limits>
#include <algorithm>
#include <iostream>

Evaluator::Evaluator() : maxQDepth(2) {
    initializePieceSquareTables();

    // init zobrist
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist;
    zobristKeys.assign(12, std::vector<uint64_t>(64));
    for (int i = 0; i < 12; ++i)
        for (int sq = 0; sq < 64; ++sq)
            zobristKeys[i][sq] = dist(rng);
    zobristSide = dist(rng);
}

void Evaluator::initializePieceSquareTables() {
    whitePawnTable = {
            0, 0, 0, 0, 0, 0, 0, 0,
            50,50,50,50,50,50,50,50,
            10,10,20,30,30,20,10,10,
            5, 5,10,25,25,10, 5, 5,
            0, 0, 0,20,20, 0, 0, 0,
            5,-5,-10, 0, 0,-10,-5, 5,
            5,10,10,-20,-20,10,10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
    };
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
    whiteRookTable = {
            0, 0, 0, 0, 0, 0, 0, 0,
            5,10,10,10,10,10,10, 5,
            -5, 0, 0, 0, 0, 0, 0,-5,
            -5, 0, 0, 0, 0, 0, 0,-5,
            -5, 0, 0, 0, 0, 0, 0,-5,
            -5, 0, 0, 0, 0, 0, 0,-5,
            -5, 0, 0, 0, 0, 0, 0,-5,
            0, 0, 0, 5, 5, 0, 0, 0
    };
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

    // mirror for black
    blackPawnTable   = std::vector<int>(whitePawnTable.rbegin(),   whitePawnTable.rend());
    blackKnightTable = std::vector<int>(whiteKnightTable.rbegin(), whiteKnightTable.rend());
    blackBishopTable = std::vector<int>(whiteBishopTable.rbegin(), whiteBishopTable.rend());
    blackRookTable   = std::vector<int>(whiteRookTable.rbegin(),   whiteRookTable.rend());
    blackQueenTable  = std::vector<int>(whiteQueenTable.rbegin(),  whiteQueenTable.rend());
    blackKingTableMG = std::vector<int>(whiteKingTableMG.rbegin(), whiteKingTableMG.rend());
    blackKingTableEG = std::vector<int>(whiteKingTableEG.rbegin(), whiteKingTableEG.rend());
}

uint64_t Evaluator::generateZobristHash(const Board& board, bool sideToMove) const {
    uint64_t h = 0;
    for (int sq = 0; sq < 64; ++sq) {
        uint64_t bit = (1ULL<<sq);
        if (board.whitePawns   & bit) h ^= zobristKeys[0][sq];
        if (board.whiteKnights & bit) h ^= zobristKeys[1][sq];
        if (board.whiteBishops & bit) h ^= zobristKeys[2][sq];
        if (board.whiteRooks   & bit) h ^= zobristKeys[3][sq];
        if (board.whiteQueens  & bit) h ^= zobristKeys[4][sq];
        if (board.whiteKings   & bit) h ^= zobristKeys[5][sq];
        if (board.blackPawns   & bit) h ^= zobristKeys[6][sq];
        if (board.blackKnights & bit) h ^= zobristKeys[7][sq];
        if (board.blackBishops & bit) h ^= zobristKeys[8][sq];
        if (board.blackRooks   & bit) h ^= zobristKeys[9][sq];
        if (board.blackQueens  & bit) h ^= zobristKeys[10][sq];
        if (board.blackKings   & bit) h ^= zobristKeys[11][sq];
    }
    if (sideToMove) h ^= zobristSide;
    return h;
}

int Evaluator::evaluateMaterial(const Board& board) const {
    auto pop = [](uint64_t b){ return __builtin_popcountll(b); };
    int m = 0;
    m += 100*pop(board.whitePawns);
    m += 320*pop(board.whiteKnights);
    m += 330*pop(board.whiteBishops);
    m += 500*pop(board.whiteRooks);
    m += 900*pop(board.whiteQueens);
    m += 20000*pop(board.whiteKings);
    m -= 100*pop(board.blackPawns);
    m -= 320*pop(board.blackKnights);
    m -= 330*pop(board.blackBishops);
    m -= 500*pop(board.blackRooks);
    m -= 900*pop(board.blackQueens);
    m -= 20000*pop(board.blackKings);
    return m;
}

int Evaluator::evaluatePositional(const Board& board) const {
    int score = 0;
    auto scan = [&](uint64_t bb, const std::vector<int>& table, int sign){
        while(bb){
            int sq = __builtin_ctzll(bb);
            score += sign * table[sq];
            bb &= bb-1;
        }
    };
    scan(board.whitePawns,   whitePawnTable,   +1);
    scan(board.whiteKnights, whiteKnightTable, +1);
    scan(board.whiteBishops, whiteBishopTable, +1);
    scan(board.whiteRooks,   whiteRookTable,   +1);
    scan(board.whiteQueens,  whiteQueenTable,  +1);
    scan(board.whiteKings,   whiteKingTableMG, +1);
    scan(board.blackPawns,   blackPawnTable,   -1);
    scan(board.blackKnights, blackKnightTable, -1);
    scan(board.blackBishops, blackBishopTable, -1);
    scan(board.blackRooks,   blackRookTable,   -1);
    scan(board.blackQueens,  blackQueenTable,  -1);
    scan(board.blackKings,   blackKingTableMG, -1);
    return score;
}

int Evaluator::evaluate(const Board& board) const {
    return evaluateMaterial(board) + evaluatePositional(board);
}

std::vector<Move> Evaluator::orderMoves(const std::vector<Move>& moves,
                                        const Board& board,
                                        bool maximizingPlayer) const
{
    std::vector<std::pair<int,Move>> scored;
    for (auto& m : moves) {
        int sc = 0;
        if (m.isCapture()) {
            char vic = board.getPieceAt(m.end);
            char att = board.getPieceAt(m.start);
            int vi = std::string("PNBRQKpnbrqk").find(vic);
            int ai = std::string("PNBRQKpnbrqk").find(att);
            sc = pieceValues[vi] - pieceValues[ai];
        }
        scored.push_back({sc,m});
    }
    std::sort(scored.begin(), scored.end(),
              [&](auto&a,auto&b){ return a.first>b.first; });
    std::vector<Move> out;
    out.reserve(moves.size());
    for (auto& p: scored) out.push_back(p.second);
    return out;
}

int Evaluator::quiescenceSearch(Board board,
                                int alpha, int beta,
                                bool maximizingPlayer,
                                int depth)
{
    if (depth >= maxQDepth)
        return evaluate(board);

    int standPat = evaluate(board);
    if (maximizingPlayer) {
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);
        auto moves = board.generateLegalMoves(Color::WHITE);
        for (auto& m : moves) {
            if (!m.isCapture()) continue;
            char vic = board.getPieceAt(m.end);
            if (vic=='K'‖vic=='k') continue;
            Board nb = board;
            nb.makeMove(m, Color::WHITE);
            alpha = std::max(alpha,
                             -quiescenceSearch(nb, -beta, -alpha, false, depth+1));
            if (alpha >= beta) break;
        }
        return alpha;
    } else {
        if (standPat <= alpha) return alpha;
        beta = std::min(beta, standPat);
        auto moves = board.generateLegalMoves(Color::BLACK);
        for (auto& m : moves) {
            if (!m.isCapture()) continue;
            char vic = board.getPieceAt(m.end);
            if (vic=='K'‖vic=='k') continue;
            Board nb = board;
            nb.makeMove(m, Color::BLACK);
            beta = std::min(beta,
                            -quiescenceSearch(nb, -beta, -alpha, true, depth+1));
            if (alpha >= beta) break;
        }
        return beta;
    }
}

std::pair<int,Move> Evaluator::alphaBeta(Board board,
                                         int depth,
                                         int alpha,
                                         int beta,
                                         bool maximizingPlayer)
{
    int alphaOrig = alpha;
    Move bestMove;
    int bestScore = maximizingPlayer
                    ? std::numeric_limits<int>::min()
                    : std::numeric_limits<int>::max();

    uint64_t h = generateZobristHash(board, maximizingPlayer);
    auto it = transpositionTable.find(h);
    if (it != transpositionTable.end()) {
        auto& e = it->second;
        if (e.depth >= depth) {
            if (e.flag==EXACT)      return {e.score,{}};
            if (e.flag==ALPHA_FLAG && e.score<=alpha) return {alpha,{}};
            if (e.flag==BETA_FLAG  && e.score>=beta)  return {beta,{}};
        }
    }

    if (depth==0) {
        return { quiescenceSearch(board, alpha, beta, maximizingPlayer, 0), {} };
    }

    auto moves = board.generateLegalMoves(
            maximizingPlayer ? Color::WHITE : Color::BLACK);
    moves = orderMoves(moves, board, maximizingPlayer);

    if (moves.empty()) {
        return maximizingPlayer ? std::make_pair(-100000, Move())
                                : std::make_pair( 100000, Move());
    }

    for (auto& m : moves) {
        Board nb = board;
        nb.makeMove(m, maximizingPlayer ? Color::WHITE : Color::BLACK);
        int score = -alphaBeta(nb, depth-1, -beta, -alpha, !maximizingPlayer).first;
        if (maximizingPlayer ? (score>bestScore) : (score<bestScore)) {
            bestScore = score;
            bestMove  = m;
        }
        if (maximizingPlayer) alpha = std::max(alpha, score);
        else                beta  = std::min(beta,  score);
        if (alpha>=beta) break;
    }

    TTEntry e{depth,bestScore,
              bestScore<=alphaOrig ? BETA_FLAG :
              bestScore>=beta      ? ALPHA_FLAG :
              EXACT};
    transpositionTable[h] = e;
    return {bestScore,bestMove};
}

Move Evaluator::iterativeDeepening(const Board& board,
                                   int maxDepth,
                                   bool maximizingPlayer)
{
    Move bestMove;
    int bestScore = 0;
    for (int d=1; d<=maxDepth; ++d) {
        auto [score, mv] = alphaBeta(board, d,
                                     std::numeric_limits<int>::min(),
                                     std::numeric_limits<int>::max(),
                                     maximizingPlayer);
        bestScore = score;
        if (mv.isValid()) bestMove = mv;
        std::cout << "Depth " << d
                  << " completed: Best score " << score
                  << ", Best move: " << bestMove.toString()
                  << "\n";
    }
    return bestMove;
}