#include "evaluator.h"
#include <bitset>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <iostream>

// Initialize piece-square tables with standard values
void PieceSquareTables::initialize() {
    // Initialize Pawn table, Knight table, etc. (as in your previous implementation)
    // (Skipping for brevity, but it's the same as what you wrote.)
}

// Initialize the Evaluator
Evaluator::Evaluator() {
    pieceSquareTables.initialize();
}

// Count the number of bits set to 1 in a bitboard (Hamming weight)
int Evaluator::countBits(uint64_t bitboard) const {
    return std::bitset<64>(bitboard).count();
}

// Evaluate material count using bitboards
int Evaluator::evaluateMaterial(const Board& board) const {
    int material = 0;

    // White material
    material += PAWN_VALUE * countBits(board.getWhitePawns());
    material += KNIGHT_VALUE * countBits(board.getWhiteKnights());
    material += BISHOP_VALUE * countBits(board.getWhiteBishops());
    material += ROOK_VALUE * countBits(board.getWhiteRooks());
    material += QUEEN_VALUE * countBits(board.getWhiteQueens());
    material += 20000 * countBits(board.getWhiteKings());

    // Black material
    material -= PAWN_VALUE * countBits(board.getBlackPawns());
    material -= KNIGHT_VALUE * countBits(board.getBlackKnights());
    material -= BISHOP_VALUE * countBits(board.getBlackBishops());
    material -= ROOK_VALUE * countBits(board.getBlackRooks());
    material -= QUEEN_VALUE * countBits(board.getBlackQueens());
    material -= 20000 * countBits(board.getBlackKings());

    return material;
}

// Evaluate piece-square tables
int Evaluator::evaluatePieceSquare(const Board& board) const {
    int score = 0;

    // Iterate through all squares for white pieces
    for (int square = 0; square < 64; ++square) {
        if (board.getWhitePawns() & (1ULL << square)) {
            score += pieceSquareTables.pawn_table[square];
        }
        if (board.getWhiteKnights() & (1ULL << square)) {
            score += pieceSquareTables.knight_table[square];
        }
        if (board.getWhiteBishops() & (1ULL << square)) {
            score += pieceSquareTables.bishop_table[square];
        }
        if (board.getWhiteRooks() & (1ULL << square)) {
            score += pieceSquareTables.rook_table[square];
        }
        if (board.getWhiteQueens() & (1ULL << square)) {
            score += pieceSquareTables.queen_table[square];
        }
        if (board.getWhiteKings() & (1ULL << square)) {
            score += pieceSquareTables.king_table[square];
        }
    }

    // Iterate through all squares for black pieces
    for (int square = 0; square < 64; ++square) {
        if (board.getBlackPawns() & (1ULL << square)) {
            score -= pieceSquareTables.pawn_table[63 - square];
        }
        if (board.getBlackKnights() & (1ULL << square)) {
            score -= pieceSquareTables.knight_table[63 - square];
        }
        if (board.getBlackBishops() & (1ULL << square)) {
            score -= pieceSquareTables.bishop_table[63 - square];
        }
        if (board.getBlackRooks() & (1ULL << square)) {
            score -= pieceSquareTables.rook_table[63 - square];
        }
        if (board.getBlackQueens() & (1ULL << square)) {
            score -= pieceSquareTables.queen_table[63 - square];
        }
        if (board.getBlackKings() & (1ULL << square)) {
            score -= pieceSquareTables.king_table[63 - square];
        }
    }

    return score;
}

// Evaluate King Safety (simplified for demonstration)
int Evaluator::evaluateKingSafety(const Board& board) const {
    // Add custom logic for king safety, e.g., checking how many pawns protect the king
    // and proximity of enemy pieces to the king.

    int score = 0;
    // Example: Evaluate pawns protecting the king
    // Add more detailed king safety logic as necessary.
    return score;
}

// Evaluate passed pawns
int Evaluator::evaluatePassedPawns(const Board& board) const {
    int score = 0;

    // White passed pawns
    uint64_t whitePawns = board.getWhitePawns();
    while (whitePawns) {
        int square = __builtin_ctzll(whitePawns); // Get index of first set bit
        whitePawns &= whitePawns - 1; // Clear least significant bit

        // Evaluate passed pawn: No opposing pawns in the same file
        if (!(board.getBlackPawns() & (1ULL << square))) {
            score += 50; // Bonus for passed pawn
        }
    }

    // Black passed pawns
    uint64_t blackPawns = board.getBlackPawns();
    while (blackPawns) {
        int square = __builtin_ctzll(blackPawns); // Get index of first set bit
        blackPawns &= blackPawns - 1;

        if (!(board.getWhitePawns() & (1ULL << square))) {
            score -= 50; // Bonus for passed pawn
        }
    }

    return score;
}

// Evaluate mobility
int Evaluator::evaluateMobility(const Board& board, Color color) const {
    // Evaluate how many legal moves the current player has.
    int mobility = generateLegalMoves(board, color).size();
    return mobility;
}

// Evaluate threats
int Evaluator::evaluateThreats(const Board& board, Color color) const {
    int score = 0;

    // Check if a player is threatening any opponent’s high-value pieces (queen, rook, etc.)
    // Implement logic to reward attacking moves.

    return score;
}

// Full evaluation function combining all components
int Evaluator::evaluate(const Board& board) const {
    int score = 0;

    // Material evaluation
    score += evaluateMaterial(board);

    // Piece-square tables evaluation
    score += evaluatePieceSquare(board);

    // King safety evaluation
    score += evaluateKingSafety(board);

    // Passed pawns evaluation
    score += evaluatePassedPawns(board);

    // Mobility evaluation
    score += evaluateMobility(board, Color::WHITE) - evaluateMobility(board, Color::BLACK);

    // Threats evaluation
    score += evaluateThreats(board, Color::WHITE) - evaluateThreats(board, Color::BLACK);

    return score;
}

// Quiescence Search for extending the search in tactical positions
int Evaluator::quiescenceSearch(Board& board, int alpha, int beta, bool maximizingPlayer) const {
    int stand_pat = evaluate(board);

    if (maximizingPlayer) {
        if (stand_pat >= beta) return beta;
        if (alpha < stand_pat) alpha = stand_pat;

        std::vector<Move> captures = generateLegalMoves(board, maximizingPlayer ? Color::WHITE : Color::BLACK);
        for (const auto& move : captures) {
            if (!move.isCapture()) continue;

            Board newBoard = board;
            newBoard.makeMove(move, maximizingPlayer ? Color::WHITE : Color::BLACK);
            int eval = -quiescenceSearch(newBoard, -beta, -alpha, !maximizingPlayer);

            if (eval >= beta) return beta;
            if (eval > alpha) alpha = eval;
        }
    } else {
        if (stand_pat <= alpha) return alpha;
        if (beta > stand_pat) beta = stand_pat;

        std::vector<Move> captures = generateLegalMoves(board, maximizingPlayer ? Color::WHITE : Color::BLACK);
        for (const auto& move : captures) {
            if (!move.isCapture()) continue;

            Board newBoard = board;
            newBoard.makeMove(move, maximizingPlayer ? Color::WHITE : Color::BLACK);
            int eval = -quiescenceSearch(newBoard, -beta, -alpha, !maximizingPlayer);

            if (eval <= alpha) return alpha;
            if (eval < beta) beta = eval;
        }
    }

    return maximizingPlayer ? alpha : beta;
}

// Implement Minimax Search with Alpha-Beta Pruning
int Evaluator::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable) const {
    if (depth == 0) {
        return quiescenceSearch(board, alpha, beta, maximizingPlayer);
    }

    uint64_t boardHash = board.currentBoardState;  // Example hash function
    if (transpositionTable.find(boardHash) != transpositionTable.end()) {
        HashEntry entry = transpositionTable[boardHash];
        if (entry.depth >= depth) {
            return entry.score;
        }
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        std::vector<Move> moves = generateLegalMoves(board, Color::WHITE);
        moveOrdering(moves, board, true);  // Order moves
        for (const auto& move : moves) {
            Board newBoard = board;
            newBoard.makeMove(move, Color::WHITE);
            int eval = minimax(newBoard, depth - 1, alpha, beta, false, transpositionTable);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        HashEntry entry = {boardHash, depth, maxEval, 0};
        transpositionTable[boardHash] = entry;
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        std::vector<Move> moves = generateLegalMoves(board, Color::BLACK);
        moveOrdering(moves, board, false);  // Order moves
        for (const auto& move : moves) {
            Board newBoard = board;
            newBoard.makeMove(move, Color::BLACK);
            int eval = minimax(newBoard, depth - 1, alpha, beta, true, transpositionTable);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        HashEntry entry = {boardHash, depth, minEval, 0};
        transpositionTable[boardHash] = entry;
        return minEval;
    }
}

// Move ordering based on heuristic (captures, threats, etc.)
int Evaluator::moveOrdering(std::vector<Move>& moves, const Board& board, bool maximizingPlayer) const {
    // Implement logic for move ordering (like MVV-LVA for captures, history heuristic, killer moves)
    // Sort moves based on how promising they are for better alpha-beta pruning.
    return 0;
}

// Generate all legal moves for a given color
std::vector<Move> Evaluator::generateLegalMoves(const Board& board, Color color) const {
    std::vector<Move> legalMoves;
    // Add logic to generate all legal moves for the specified color
    return legalMoves;
}
