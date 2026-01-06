#include "evaluator.h"
#include <algorithm>
#include <random>

static constexpr int MATE_SCORE = 100000;

Evaluator::Evaluator() {
    initializePieceSquareTables();

    std::mt19937_64 rng(123456789);
    std::uniform_int_distribution<uint64_t> dist;

    zobristKeys.assign(12, std::vector<uint64_t>(64));

    for (auto& pieceType : zobristKeys) {
        for (auto& squareKey : pieceType) {
            squareKey = dist(rng);
        }
    }
    zobristSide = dist(rng);
}

uint64_t Evaluator::generateZobristHash(const Board& board, bool sideToMove) const {
    uint64_t h = 0;

    for (int sq = 0; sq < 64; ++sq) {
        uint64_t bit = 1ULL << sq;

        for (int pi = 0; pi < 6; ++pi) {
            if (board.pieceBB(Color::WHITE, static_cast<Board::PieceIndex>(pi)) & bit) {
                h ^= zobristKeys[pi][sq];
                break;
            }
        }

        for (int pi = 0; pi < 6; ++pi) {
            if (board.pieceBB(Color::BLACK, static_cast<Board::PieceIndex>(pi)) & bit) {
                h ^= zobristKeys[pi + 6][sq];
                break;
            }
        }
    }

    if (sideToMove) {
        h ^= zobristSide;
    }

    return h;
}

int Evaluator::evaluate(const Board& board, Color stm) const {
    if (board.isCheckmate(stm)) return -MATE_SCORE;

    if (board.isStalemate(stm) || board.isFiftyMoveDraw() ||
        board.isThreefoldRepetition() || board.isInsufficientMaterial()) {
        return 0;
    }

    int score = evaluateMaterial(board) + evaluatePositional(board);
    return (stm == Color::WHITE ? score : -score);
}

int Evaluator::evaluateTerminal(const Board& board, Color stm) const {
    if (board.isCheckmate(stm)) return -MATE_SCORE;
    return 0;
}

int Evaluator::evaluateMaterial(const Board& board) const {
    auto countSetBits = [](uint64_t b) { return __builtin_popcountll(b); };

    int score = 0;
    for (int pt = 0; pt < PST_COUNT; ++pt) {
        int whiteCount = countSetBits(board.pieceBB(Color::WHITE, static_cast<Board::PieceIndex>(pt)));
        int blackCount = countSetBits(board.pieceBB(Color::BLACK, static_cast<Board::PieceIndex>(pt)));

        score += pieceValues[pt] * (whiteCount - blackCount);
    }
    return score;
}

int Evaluator::evaluatePositional(const Board& board) const {
    int score = 0;

    auto applyPST = [&](uint64_t bb, const std::vector<int>& table, int sign) {
        while (bb) {
            int sq = __builtin_ctzll(bb);
            score += sign * table[sq];
            bb &= bb - 1;
        }
    };

    applyPST(board.pieceBB(Color::WHITE, Board::PAWN), whitePawnTable, 1);
    applyPST(board.pieceBB(Color::WHITE, Board::KNIGHT), whiteKnightTable, 1);
    applyPST(board.pieceBB(Color::WHITE, Board::BISHOP), whiteBishopTable, 1);
    applyPST(board.pieceBB(Color::WHITE, Board::ROOK), whiteRookTable, 1);
    applyPST(board.pieceBB(Color::WHITE, Board::QUEEN), whiteQueenTable, 1);
    applyPST(board.pieceBB(Color::WHITE, Board::KING), whiteKingTableMG, 1);

    applyPST(board.pieceBB(Color::BLACK, Board::PAWN), blackPawnTable, -1);
    applyPST(board.pieceBB(Color::BLACK, Board::KNIGHT), blackKnightTable, -1);
    applyPST(board.pieceBB(Color::BLACK, Board::BISHOP), blackBishopTable, -1);
    applyPST(board.pieceBB(Color::BLACK, Board::ROOK), blackRookTable, -1);
    applyPST(board.pieceBB(Color::BLACK, Board::QUEEN), blackQueenTable, -1);
    applyPST(board.pieceBB(Color::BLACK, Board::KING), blackKingTableMG, -1);

    return score;
}

void Evaluator::initializePieceSquareTables() {
    whitePawnTable = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5,
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    whiteKnightTable = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    whiteBishopTable = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    whiteRookTable = {
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 10, 10, 10, 10, 10, 10, 5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        0, 0, 0, 5, 5, 0, 0, 0
    };

    whiteQueenTable = {
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        0, 0, 5, 5, 5, 5, 0, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20
    };

    whiteKingTableMG = {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        20, 20, 0, 0, 0, 0, 20, 20,
        20, 30, 10, 0, 0, 10, 30, 20
    };

    whiteKingTableEG = whiteKingTableMG;

    blackPawnTable.assign(whitePawnTable.rbegin(), whitePawnTable.rend());
    blackKnightTable.assign(whiteKnightTable.rbegin(), whiteKnightTable.rend());
    blackBishopTable.assign(whiteBishopTable.rbegin(), whiteBishopTable.rend());
    blackRookTable.assign(whiteRookTable.rbegin(), whiteRookTable.rend());
    blackQueenTable.assign(whiteQueenTable.rbegin(), whiteQueenTable.rend());
    blackKingTableMG.assign(whiteKingTableMG.rbegin(), whiteKingTableMG.rend());
    blackKingTableEG.assign(whiteKingTableEG.rbegin(), whiteKingTableEG.rend());
}
