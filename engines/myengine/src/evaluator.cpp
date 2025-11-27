#include "evaluator.h"
#include <algorithm>
#include <iostream>

static constexpr int  MATE_SCORE = 100000;

// Zero-initialized fallback PST:
const int Evaluator::pst[Evaluator::PST_COUNT][64] = {{0}};

Evaluator::Evaluator(unsigned maxQDepth_)
  : maxQDepth(maxQDepth_)
{
    initializePieceSquareTables();

    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    zobristKeys.assign(12, std::vector<uint64_t>(64));
    for (auto &tbl : zobristKeys)
        for (auto &k : tbl)
            k = dist(rng);
    zobristSide = dist(rng);
}

void Evaluator::initializePieceSquareTables() {
    whitePawnTable = {
        0,0,0,0,0,0,0,0,
        50,50,50,50,50,50,50,50,
        10,10,20,30,30,20,10,10,
        5,5,10,25,25,10,5,5,
        0,0,0,20,20,0,0,0,
        5,-5,-10,0,0,-10,-5,5,
        5,10,10,-20,-20,10,10,5,
        0,0,0,0,0,0,0,0
    };
    whiteKnightTable = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,0,0,0,0,-20,-40,
        -30,0,10,15,15,10,0,-30,
        -30,5,15,20,20,15,5,-30,
        -30,0,15,20,20,15,0,-30,
        -30,5,10,15,15,10,5,-30,
        -40,-20,0,5,5,0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    };
    whiteBishopTable = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,0,0,0,0,0,0,-10,
        -10,0,5,10,10,5,0,-10,
        -10,5,5,10,10,5,5,-10,
        -10,0,10,10,10,10,0,-10,
        -10,10,10,10,10,10,10,-10,
        -10,5,0,0,0,0,5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };
    whiteRookTable = {
        0,0,0,0,0,0,0,0,
        5,10,10,10,10,10,10,5,
        -5,0,0,0,0,0,0,-5,
        -5,0,0,0,0,0,0,-5,
        -5,0,0,0,0,0,0,-5,
        -5,0,0,0,0,0,0,-5,
        -5,0,0,0,0,0,0,-5,
        0,0,0,5,5,0,0,0
    };
    whiteQueenTable = {
        -20,-10,-10,-5,-5,-10,-10,-20,
        -10,0,0,0,0,0,0,-10,
        -10,0,5,5,5,5,0,-10,
        -5,0,5,5,5,5,0,-5,
        0,0,5,5,5,5,0,-5,
        -10,5,5,5,5,5,0,-10,
        -10,0,5,0,0,0,0,-10,
        -20,-10,-10,-5,-5,-10,-10,-20
    };
    whiteKingTableMG = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20,20,0,0,0,0,20,20,
        20,30,10,0,0,10,30,20
    };
    whiteKingTableEG = {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,0,0,-10,-20,-30,
        -30,-10,20,30,30,20,-10,-30,
        -30,-10,30,40,40,30,-10,-30,
        -30,-10,30,40,40,30,-10,-30,
        -30,-10,20,30,30,20,-10,-30,
        -30,-30,0,0,0,0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    };

    blackPawnTable   = { whitePawnTable.rbegin(),   whitePawnTable.rend() };
    blackKnightTable = { whiteKnightTable.rbegin(), whiteKnightTable.rend() };
    blackBishopTable = { whiteBishopTable.rbegin(), whiteBishopTable.rend() };
    blackRookTable   = { whiteRookTable.rbegin(),   whiteRookTable.rend() };
    blackQueenTable  = { whiteQueenTable.rbegin(),  whiteQueenTable.rend() };
    blackKingTableMG = { whiteKingTableMG.rbegin(), whiteKingTableMG.rend() };
    blackKingTableEG = { whiteKingTableEG.rbegin(), whiteKingTableEG.rend() };
}

uint64_t Evaluator::generateZobristHash(const Board &board,
    bool sideToMove) const {
    uint64_t h = 0;
    for (int sq = 0; sq < 64; ++sq) {
        uint64_t bit = 1ULL << sq;
        // White pieces 0–5, black 6–11
        for (int pi = 0; pi < 6; ++pi) {
            if (board.pieceBB(Color::WHITE, static_cast<Board::PieceIndex>(pi)) & bit)
                h ^= zobristKeys[pi][sq];
            if (board.pieceBB(Color::BLACK, static_cast<Board::PieceIndex>(pi)) & bit)
                h ^= zobristKeys[pi+6][sq];
        }
    }
    if (sideToMove) h ^= zobristSide;
    return h;
}

int Evaluator::evaluateMaterial(const Board &board) const {
    auto pop = [](uint64_t b){ return __builtin_popcountll(b); };
    int m = 0;
    for (int pt = 0; pt < PST_COUNT; ++pt) {
        m += pieceValues[pt] * pop(board.pieceBB(Color::WHITE, static_cast<Board::PieceIndex>(pt)));
        m -= pieceValues[pt] * pop(board.pieceBB(Color::BLACK, static_cast<Board::PieceIndex>(pt)));
    }
    return m;
}

int Evaluator::evaluatePositional(const Board &board) const {
    int score = 0;
    auto scan = [&](uint64_t bb, const std::vector<int> &tbl, int sign){
        while (bb) {
            int sq = __builtin_ctzll(bb);
            score += sign * tbl[sq];
            bb &= bb - 1;
        }
    };

    scan(board.pieceBB(Color::WHITE, Board::PAWN),   whitePawnTable,   +1);
    scan(board.pieceBB(Color::WHITE, Board::KNIGHT), whiteKnightTable, +1);
    scan(board.pieceBB(Color::WHITE, Board::BISHOP), whiteBishopTable, +1);
    scan(board.pieceBB(Color::WHITE, Board::ROOK),   whiteRookTable,   +1);
    scan(board.pieceBB(Color::WHITE, Board::QUEEN),  whiteQueenTable,  +1);
    scan(board.pieceBB(Color::WHITE, Board::KING),   whiteKingTableMG, +1);

    scan(board.pieceBB(Color::BLACK, Board::PAWN),   blackPawnTable,   -1);
    scan(board.pieceBB(Color::BLACK, Board::KNIGHT), blackKnightTable, -1);
    scan(board.pieceBB(Color::BLACK, Board::BISHOP), blackBishopTable, -1);
    scan(board.pieceBB(Color::BLACK, Board::ROOK),   blackRookTable,   -1);
    scan(board.pieceBB(Color::BLACK, Board::QUEEN),  blackQueenTable,  -1);
    scan(board.pieceBB(Color::BLACK, Board::KING),   blackKingTableMG, -1);

    return score;
}

int Evaluator::evaluate(const Board &board, Color stm) const {
    if (board.isCheckmate(stm)) return -MATE_SCORE;
    if ((board.isStalemate(stm) || board.isStalemate(opponent(stm)))
     || board.isFiftyMoveDraw()
     || board.isThreefoldRepetition()
     || board.isInsufficientMaterial())
        return 0;
    int s = evaluateMaterial(board) + evaluatePositional(board);
    return (stm == Color::WHITE ? +s : -s);
}

int Evaluator::evaluateTerminal(const Board &board, Color stm) const {
    if (board.isCheckmate(stm)) return -MATE_SCORE;
    if (board.isStalemate(stm)
     || board.isFiftyMoveDraw()
     || board.isThreefoldRepetition()
     || board.isInsufficientMaterial())
        return 0;
    return evaluate(board, stm);
}

std::vector<Move> Evaluator::orderMoves(std::vector<Move> moves,
                                        bool maximizingPlayer) {
    std::stable_sort(moves.begin(), moves.end(),
                     [](const Move &a, const Move &b){
                         return a.isCapture() && !b.isCapture();
                     });
    return moves;
}

int Evaluator::quiescenceSearch(Board board,
                                int alpha, int beta,
                                bool maximizingPlayer,
                                int depth) {
    if (depth >= maxQDepth)
        return evaluate(board, board.sideToMove());

    int standPat = evaluate(board, board.sideToMove());
    if (maximizingPlayer) {
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);
    } else {
        if (standPat <= alpha) return alpha;
        beta = std::min(beta, standPat);
    }

    for (auto &m : board.generateLegalMoves()) {
        if (!m.isCapture()) continue;
        board.makeMove(m);
        int score = -quiescenceSearch(board, -beta, -alpha,
                                      !maximizingPlayer, depth+1);
        board.unmakeMove();
        if (maximizingPlayer) {
            if (score >= beta) return beta;
            alpha = std::max(alpha, score);
        } else {
            if (score <= alpha) return alpha;
            beta = std::min(beta, score);
        }
    }
    return maximizingPlayer ? alpha : beta;
}

std::pair<int,Move> Evaluator::alphaBeta(Board board,
                                         int depth,
                                         int alpha, int beta,
                                         bool maximizingPlayer) {
    int alphaOrig = alpha;
    Move bestMove;
    int bestScore = maximizingPlayer
                    ? std::numeric_limits<int>::min()
                    : std::numeric_limits<int>::max();

    uint64_t h = generateZobristHash(board, maximizingPlayer);
    auto it = transpositionTable.find(h);
    if (it != transpositionTable.end()) {
        const auto &e = it->second;
        if (e.depth >= depth) {
            if (e.flag == EXACT)       return {e.score, {}};
            if (e.flag == ALPHA_FLAG && e.score <= alpha) return {alpha, {}};
            if (e.flag == BETA_FLAG  && e.score >= beta)  return {beta, {}};
        }
    }

    if (depth == 0) {
        return { quiescenceSearch(board, alpha, beta, maximizingPlayer, 0), {} };
    }

    auto moves = orderMoves(board.generateLegalMoves(), maximizingPlayer);
    if (moves.empty()) {
        // no legal moves = either checkmate or stalemate
        int ms = maximizingPlayer ? -MATE_SCORE : +MATE_SCORE;
        return {ms, {}};
    }

    for (auto &m : moves) {
        board.makeMove(m);
        auto [score,_] = alphaBeta(board, depth-1,
                                   -beta, -alpha,
                                   !maximizingPlayer);
        score = -score;
        board.unmakeMove();

        if ((maximizingPlayer && score > bestScore) ||
            (!maximizingPlayer && score < bestScore)) {
            bestScore = score;
            bestMove  = m;
        }
        if (maximizingPlayer) alpha = std::max(alpha, score);
        else                beta  = std::min(beta,  score);
        if (alpha >= beta) break;
    }

    TTEntry e{depth, bestScore,
              bestScore <= alphaOrig ? BETA_FLAG :
              bestScore >= beta      ? ALPHA_FLAG :
              EXACT};
    transpositionTable[h] = e;

    return {bestScore, bestMove};
}

Move Evaluator::iterativeDeepening(const Board &board,
                                   int maxDepth,
                                   bool maximizingPlayer) {
    Move bestMove;
    for (int d = 1; d <= maxDepth; ++d) {
        auto [score,mv] = alphaBeta(board, d,
                                    std::numeric_limits<int>::min(),
                                    std::numeric_limits<int>::max(),
                                    maximizingPlayer);
        if (mv.isValid()) bestMove = mv;
        std::cout << "Depth " << d
                  << " → score " << score
                  << ", move " << bestMove.toString()
                  << "\n";
    }
    return bestMove;
}
