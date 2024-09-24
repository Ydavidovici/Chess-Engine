#include "evaluator.h"
#include <iostream>
#include <bitset>
#include <algorithm>
#include <random>
#include <fstream>
#include <cmath>

// Initialize piece-square tables with standard values
void PieceSquareTables::initialize() {
    // White Pawn Table
    int pawn_init[64] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 10, 20, 30, 30, 20, 10, 10,
            5, 5, 10, 25, 25, 10, 5, 5,
            0, 0, 0, 20, 20, 0, 0, 0,
            5, -5, -10, 0, 0, -10, -5, 5,
            5, 10, 10, -20, -20, 10, 10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
    };
    std::copy(std::begin(pawn_init), std::end(pawn_init), pawn_table);

    // Black Pawn Table (mirror of white)
    for (int i = 0; i < 64; ++i) {
        pawn_table[i] = pawn_init[63 - i];
    }

    // White Knight Table
    int knight_init[64] = {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20, 0, 0, 0, 0, -20, -40,
            -30, 0, 10, 15, 15, 10, 0, -30,
            -30, 5, 15, 20, 20, 15, 5, -30,
            -30, 0, 15, 20, 20, 15, 0, -30,
            -30, 5, 10, 15, 15, 10, 5, -30,
            -40, -20, 0, 5, 5, 0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
    };
    std::copy(std::begin(knight_init), std::end(knight_init), knight_table);

    // Black Knight Table (mirror of white)
    for (int i = 0; i < 64; ++i) {
        knight_table[i] = knight_init[63 - i];
    }

    // White Bishop Table
    int bishop_init[64] = {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 10, 10, 5, 0, -10,
            -10, 5, 5, 10, 10, 5, 5, -10,
            -10, 0, 10, 10, 10, 10, 0, -10,
            -10, 10, 10, 10, 10, 10, 10, -10,
            -10, 5, 0, 0, 0, 0, 5, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
    };
    std::copy(std::begin(bishop_init), std::end(bishop_init), bishop_table);

    // Black Bishop Table (mirror of white)
    for (int i = 0; i < 64; ++i) {
        bishop_table[i] = bishop_init[63 - i];
    }

    // White Rook Table
    int rook_init[64] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, 10, 10, 10, 10, 5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            0, 0, 0, 5, 5, 0, 0, 0
    };
    std::copy(std::begin(rook_init), std::end(rook_init), rook_table);

    // Black Rook Table (mirror of white)
    for (int i = 0; i < 64; ++i) {
        rook_table[i] = rook_init[63 - i];
    }

    // White Queen Table
    int queen_init[64] = {
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -5, 0, 5, 5, 5, 5, 0, -5,
            0, 0, 5, 5, 5, 5, 0, -5,
            -10, 5, 5, 5, 5, 5, 0, -10,
            -10, 0, 5, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20
    };
    std::copy(std::begin(queen_init), std::end(queen_init), queen_table);

    // Black Queen Table (mirror of white)
    for (int i = 0; i < 64; ++i) {
        queen_table[i] = queen_init[63 - i];
    }

    // White King Table (Middlegame)
    int king_init_mg[64] = {
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            20, 20, 0, 0, 0, 0, 20, 20,
            20, 30, 10, 0, 0, 10, 30, 20
    };
    std::copy(std::begin(king_init_mg), std::end(king_init_mg), king_table_mg);

    // Black King Table (Middlegame, mirror of white)
    for (int i = 0; i < 64; ++i) {
        king_table_mg[i] = king_init_mg[63 - i];
    }

    // White King Table (Endgame)
    int king_init_eg[64] = {
            -50, -40, -30, -20, -20, -30, -40, -50,
            -30, -20, -10, 0, 0, -10, -20, -30,
            -30, -10, 20, 30, 30, 20, -10, -30,
            -30, -10, 30, 40, 40, 30, -10, -30,
            -30, -10, 30, 40, 40, 30, -10, -30,
            -30, -10, 20, 30, 30, 20, -10, -30,
            -30, -30, 0, 0, 0, 0, -30, -30,
            -50, -30, -30, -30, -30, -30, -30, -50
    };
    std::copy(std::begin(king_init_eg), std::end(king_init_eg), king_table_eg);

    // Black King Table (Endgame, mirror of white)
    for (int i = 0; i < 64; ++i) {
        king_table_eg[i] = king_init_eg[63 - i];
    }
}

// Constructor
Evaluator::Evaluator() {
    pieceSquareTables.initialize();
    initializeZobristKeys();
}

// Initialize Zobrist hashing keys
void Evaluator::initializeZobristKeys() {
    std::mt19937_64 rng(0); // Fixed seed for reproducibility
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    for (int piece = 0; piece < 12; ++piece) {
        for (int square = 0; square < 64; ++square) {
            zobristKeys[piece][square] = dist(rng);
        }
    }
    zobristSide = dist(rng);
}

// Generate Zobrist hash for the current board
uint64_t Evaluator::generateZobristHash(const Board& board, bool sideToMove) const {
    uint64_t hash = 0;
    // White pieces
    for (int square = 0; square < 64; ++square) {
        if (board.getWhitePawns() & (1ULL << square)) hash ^= zobristKeys[0][square];
        if (board.getWhiteKnights() & (1ULL << square)) hash ^= zobristKeys[1][square];
        if (board.getWhiteBishops() & (1ULL << square)) hash ^= zobristKeys[2][square];
        if (board.getWhiteRooks() & (1ULL << square)) hash ^= zobristKeys[3][square];
        if (board.getWhiteQueens() & (1ULL << square)) hash ^= zobristKeys[4][square];
        if (board.getWhiteKings() & (1ULL << square)) hash ^= zobristKeys[5][square];
    }
    // Black pieces
    for (int square = 0; square < 64; ++square) {
        if (board.getBlackPawns() & (1ULL << square)) hash ^= zobristKeys[6][square];
        if (board.getBlackKnights() & (1ULL << square)) hash ^= zobristKeys[7][square];
        if (board.getBlackBishops() & (1ULL << square)) hash ^= zobristKeys[8][square];
        if (board.getBlackRooks() & (1ULL << square)) hash ^= zobristKeys[9][square];
        if (board.getBlackQueens() & (1ULL << square)) hash ^= zobristKeys[10][square];
        if (board.getBlackKings() & (1ULL << square)) hash ^= zobristKeys[11][square];
    }
    // Side to move
    if (sideToMove) {
        hash ^= zobristSide;
    }
    return hash;
}

// Evaluate material count using bitboards
int Evaluator::evaluateMaterial(const Board& board) const {
    int material = 0;

    // White material
    material += 100 * __builtin_popcountll(board.getWhitePawns());
    material += 320 * __builtin_popcountll(board.getWhiteKnights());
    material += 330 * __builtin_popcountll(board.getWhiteBishops());
    material += 500 * __builtin_popcountll(board.getWhiteRooks());
    material += 900 * __builtin_popcountll(board.getWhiteQueens());
    material += 20000 * __builtin_popcountll(board.getWhiteKings());

    // Black material
    material -= 100 * __builtin_popcountll(board.getBlackPawns());
    material -= 320 * __builtin_popcountll(board.getBlackKnights());
    material -= 330 * __builtin_popcountll(board.getBlackBishops());
    material -= 500 * __builtin_popcountll(board.getBlackRooks());
    material -= 900 * __builtin_popcountll(board.getBlackQueens());
    material -= 20000 * __builtin_popcountll(board.getBlackKings());

    return material;
}

// Evaluate piece-square tables
int Evaluator::evaluatePieceSquare(const Board& board) const {
    int score = 0;

    // White pieces
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
            // Decide between middlegame and endgame king tables based on game phase
            // For simplicity, using middlegame table
            score += pieceSquareTables.king_table_mg[square];
        }
    }

    // Black pieces
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
            // Decide between middlegame and endgame king tables based on game phase
            // For simplicity, using middlegame table
            score -= pieceSquareTables.king_table_mg[63 - square];
        }
    }

    return score;
}

// Evaluate king safety (simplified)
int Evaluator::evaluateKingSafety(const Board& board) const {
    // Simplistic king safety: count pawns near the king
    int score = 0;

    // White King
    uint64_t whiteKing = board.getWhiteKings();
    if (whiteKing) {
        int kingPos = __builtin_ctzll(whiteKing);
        // Define a mask around the king (one square radius)
        uint64_t mask = 0ULL;
        for (int d = -1; d <= 1; d++) {
            for (int e = -1; e <= 1; e++) {
                int rank = kingPos / 8 + d;
                int file = kingPos % 8 + e;
                if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                    mask |= (1ULL << (rank * 8 + file));
                }
            }
        }
        mask &= ~whiteKing; // Exclude the king's own square
        score += 10 * __builtin_popcountll(board.getWhitePawns() & mask);
    }

    // Black King
    uint64_t blackKing = board.getBlackKings();
    if (blackKing) {
        int kingPos = __builtin_ctzll(blackKing);
        // Define a mask around the king (one square radius)
        uint64_t mask = 0ULL;
        for (int d = -1; d <= 1; d++) {
            for (int e = -1; e <= 1; e++) {
                int rank = kingPos / 8 + d;
                int file = kingPos % 8 + e;
                if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                    mask |= (1ULL << (rank * 8 + file));
                }
            }
        }
        mask &= ~blackKing; // Exclude the king's own square
        score -= 10 * __builtin_popcountll(board.getBlackPawns() & mask);
    }

    return score;
}

// Evaluate passed pawns
int Evaluator::evaluatePassedPawns(const Board& board) const {
    int score = 0;

    // White passed pawns
    uint64_t whitePawns = board.getWhitePawns();
    while (whitePawns) {
        int square = __builtin_ctzll(whitePawns);
        whitePawns &= whitePawns - 1;
        bool passed = true;
        int rank = square / 8;
        int file = square % 8;
        // Check if any black pawns are in the same file or adjacent files ahead
        for (int r = rank + 1; r < 8 && passed; r++) {
            for (int f = std::max(0, file - 1); f <= std::min(7, file + 1) && passed; f++) {
                int pos = r * 8 + f;
                if (board.getBlackPawns() & (1ULL << pos)) {
                    passed = false;
                }
            }
        }
        if (passed) {
            score += 50; // Bonus for passed pawn
        }
    }

    // Black passed pawns
    uint64_t blackPawns = board.getBlackPawns();
    while (blackPawns) {
        int square = __builtin_ctzll(blackPawns);
        blackPawns &= blackPawns - 1;
        bool passed = true;
        int rank = square / 8;
        int file = square % 8;
        // Check if any white pawns are in the same file or adjacent files behind
        for (int r = rank - 1; r >= 0 && passed; r--) {
            for (int f = std::max(0, file - 1); f <= std::min(7, file + 1) && passed; f++) {
                int pos = r * 8 + f;
                if (board.getWhitePawns() & (1ULL << pos)) {
                    passed = false;
                }
            }
        }
        if (passed) {
            score -= 50; // Bonus for passed pawn
        }
    }

    return score;
}

// Evaluate mobility
int Evaluator::evaluateMobility(const Board& board, Color color) const {
    // Number of legal moves available
    return board.generateLegalMoves(color).size();
}

// Evaluate threats (simplified)
int Evaluator::evaluateThreats(const Board& board, Color color) const {
    int score = 0;

    // Identify high-value opponent pieces
    uint64_t opponentPieces = (color == WHITE) ?
                              (board.getBlackQueens() | board.getBlackRooks()) :
                              (board.getWhiteQueens() | board.getWhiteRooks());

    // Generate all possible moves
    std::vector<Move> moves = board.generateLegalMoves(color);
    for (const auto& move : moves) {
        if (opponentPieces & (1ULL << move.getEndPosition())) {
            score += 30; // Bonus for threatening high-value piece
        }
    }

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
    score += evaluateMobility(board, WHITE) - evaluateMobility(board, BLACK);

    // Threats evaluation
    score += evaluateThreats(board, WHITE) - evaluateThreats(board, BLACK);

    return score;
}

// Quiescence Search for tactical evaluation
int Evaluator::quiescenceSearch(Board& board, int alpha, int beta, bool maximizingPlayer) const {
    int stand_pat = evaluate(board);

    if (maximizingPlayer) {
        if (stand_pat >= beta) return beta;
        if (alpha < stand_pat) alpha = stand_pat;

        std::vector<Move> captures = board.generateLegalMoves(WHITE);
        for (const auto& move : captures) {
            if (!move.isCapture()) continue;

            Board newBoard = board;
            if (!newBoard.makeMove(move, WHITE)) continue; // Invalid move handling
            int eval = -quiescenceSearch(newBoard, -beta, -alpha, false);

            if (eval >= beta) return beta;
            if (eval > alpha) alpha = eval;
        }
    } else {
        if (stand_pat <= alpha) return alpha;
        if (beta > stand_pat) beta = stand_pat;

        std::vector<Move> captures = board.generateLegalMoves(BLACK);
        for (const auto& move : captures) {
            if (!move.isCapture()) continue;

            Board newBoard = board;
            if (!newBoard.makeMove(move, BLACK)) continue; // Invalid move handling
            int eval = -quiescenceSearch(newBoard, -beta, -alpha, true);

            if (eval <= alpha) return alpha;
            if (eval < beta) beta = eval;
        }
    }

    return maximizingPlayer ? alpha : beta;
}

// Principal Variation Search (integrated into search)
int Evaluator::search(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    if (depth == 0) return quiescenceSearch(board, alpha, beta, maximizingPlayer);

    uint64_t hash = generateZobristHash(board, maximizingPlayer);

    // Probe transposition table
    int ttScore;
    if (probeTranspositionTable(hash, depth, alpha, beta, ttScore)) {
        return ttScore;
    }

    // Null move pruning
    if (shouldNullMovePrune(board, depth, beta, maximizingPlayer)) {
        Board nullBoard = board;
        // Implement a null move (skip player's turn)
        // For simplicity, assume a function nullMove() exists which switches the side to move
        // Here, we emulate a null move by simply toggling the side to move without making any actual move
        // This requires modifying the Board class to handle side-to-move, which is not shown here
        // Therefore, we'll skip actual null move implementation
        // int nullEval = -search(nullBoard, depth - 2, -beta, -alpha, !maximizingPlayer);
        // if (nullEval >= beta) return beta;
        // Placeholder: not implemented
    }

    std::vector<Move> moves = board.generateLegalMoves(maximizingPlayer ? WHITE : BLACK);
    moveOrdering(moves, board, maximizingPlayer);

    if (moves.empty()) {
        // Checkmate or stalemate
        // For simplicity, return a large positive or negative score
        // In practice, differentiate between checkmate (high positive/negative) and stalemate (0)
        bool inCheck = isCheck(board, maximizingPlayer ? WHITE : BLACK);
        if (inCheck) {
            return maximizingPlayer ? -100000 : 100000; // Checkmate
        }
        return 0; // Stalemate
    }

    bool isPVNode = true;
    int bestScore = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (const auto& move : moves) {
        Board newBoard = board;
        if (!newBoard.makeMove(move, maximizingPlayer ? WHITE : BLACK)) continue; // Invalid move handling
        int score;

        if (isPVNode) {
            score = -search(newBoard, depth - 1, -beta, -alpha, !maximizingPlayer);
            isPVNode = false;
        } else {
            // Research window
            score = -search(newBoard, depth - 1, -alpha - 1, -alpha, !maximizingPlayer);
            if (score > alpha && score < beta) {
                // Re-search with full window
                score = -search(newBoard, depth - 1, -beta, -alpha, !maximizingPlayer);
            }
        }

        if (maximizingPlayer) {
            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
        } else {
            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
        }

        if (score >= beta) {
            storeTransposition(hash, depth, beta, BETA);
            return beta;
        }
    }

    storeTransposition(hash, depth, bestScore, EXACT);
    return bestScore;
}

// Iterative Deepening
int Evaluator::iterativeDeepening(Board& board, int maxDepth, bool maximizingPlayer) {
    int bestScore = 0;
    for (int depth = 1; depth <= maxDepth; ++depth) {
        bestScore = search(board, depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), maximizingPlayer);
        // Optionally, store the best move found at each depth
        std::cout << "Depth " << depth << " completed with score: " << bestScore << std::endl;
    }
    return bestScore;
}

// Move ordering based on heuristic (captures first, then others)
void Evaluator::moveOrdering(std::vector<Move>& moves, const Board& board, bool maximizingPlayer) const {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) -> bool {
        // Prioritize captures over normal moves
        if (a.isCapture() && !b.isCapture()) return true;
        if (b.isCapture() && !a.isCapture()) return false;

        // Further heuristics: MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
        // Assign scores based on the piece types involved in the capture
        // For simplicity, using material values

        auto getPieceValue = [&](const Move& move) -> int {
            // Determine the value of the captured piece
            // This requires access to the board's piece bitboards
            // Simplified: assuming the move captures a piece based on the end position
            // In practice, you would need to query the board for the piece at endPosition
            // Here, we'll use a placeholder value
            // Example values: Pawn=100, Knight=320, Bishop=330, Rook=500, Queen=900
            // Negative values for black pieces
            // Placeholder implementation:
            // This requires the Board class to have a method to get the piece type at a square
            // For simplicity, returning a fixed value
            return 100; // Placeholder
        };

        int aScore = a.isCapture() ? getPieceValue(a) : 0;
        int bScore = b.isCapture() ? getPieceValue(b) : 0;
        return aScore > bScore;
    });
}

// Probe the transposition table for an existing entry
bool Evaluator::probeTranspositionTable(uint64_t hash, int depth, int alpha, int beta, int& score) const {
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end()) {
        const HashEntry& entry = it->second;
        if (entry.depth >= depth) {
            if (entry.flag == EXACT) {
                score = entry.score;
                return true;
            }
            if (entry.flag == ALPHA && entry.score <= alpha) {
                score = alpha;
                return true;
            }
            if (entry.flag == BETA && entry.score >= beta) {
                score = beta;
                return true;
            }
        }
    }
    return false;
}

// Store an entry in the transposition table
void Evaluator::storeTransposition(uint64_t hash, int depth, int score, int flag) {
    HashEntry entry = { hash, depth, score, flag };
    transpositionTable[hash] = entry;
}

// Null Move Pruning
bool Evaluator::shouldNullMovePrune(const Board& board, int depth, int beta, bool maximizingPlayer) const {
    // Null move pruning heuristic:
    // Perform a null move (skip a move) and search with reduced depth
    // If the null move search returns a value >= beta, prune this branch
    // Conditions to apply null move pruning:
    // - Depth is sufficient (e.g., depth > 2)
    // - Not in zugzwang (simplified check)
    // - Not in check

    if (depth <= 2) return false;

    // Simplified: check if the current player is not in check
    if (isCheck(board, maximizingPlayer ? WHITE : BLACK)) return false;

    // Perform null move (skip turn)
    // For simplicity, assume we can switch the side to move
    Board nullBoard = board;
    // Implement null move: switch the side to move without making any move
    // This requires modifying the Board class to handle side-to-move, which is not shown here
    // Placeholder: not implemented

    // int nullEval = -search(nullBoard, depth - 2, -beta, -alpha, !maximizingPlayer);
    // For demonstration, we'll assume a high evaluation
    int nullEval = maximizingPlayer ? -100000 : 100000;

    if (nullEval >= beta) return true;

    return false;
}

// Simulate games for backtesting (placeholder)
int Evaluator::simulateGames() const {
    // Implement game simulation logic
    // This could involve self-play or playing against predefined opponents
    // For simplicity, return a random score
    return rand() % 1000;
}

// Tune parameters based on game results
void Evaluator::tuneParameters(const std::vector<GameResult>& gameResults) {
    // Implement backtesting and parameter tuning based on game results
    // Adjust piece-square tables accordingly
    // For simplicity, adjust by a fixed factor based on win rates

    // Calculate win rates
    int whiteWins = 0, blackWins = 0, draws = 0;
    for (const auto& result : gameResults) {
        if (result.whiteWin) whiteWins++;
        else if (result.blackWin) blackWins++;
        else draws++;
    }

    double whiteWinRate = gameResults.empty() ? 0.0 : static_cast<double>(whiteWins) / gameResults.size();
    double blackWinRate = gameResults.empty() ? 0.0 : static_cast<double>(blackWins) / gameResults.size();

    // Simple heuristic: if white win rate is high, increase white's piece-square values
    // Similarly for black
    double adjustmentFactorWhite = 1.0 + (whiteWinRate - 0.5) * 0.1; // Adjust between 0.9 and 1.1
    double adjustmentFactorBlack = 1.0 + (blackWinRate - 0.5) * 0.1; // Adjust between 0.9 and 1.1

    // Apply adjustments
    for (int i = 0; i < 64; ++i) {
        pieceSquareTables.pawn_table[i] = static_cast<int>(pieceSquareTables.pawn_table[i] * adjustmentFactorWhite);
        pieceSquareTables.knight_table[i] = static_cast<int>(pieceSquareTables.knight_table[i] * adjustmentFactorWhite);
        pieceSquareTables.bishop_table[i] = static_cast<int>(pieceSquareTables.bishop_table[i] * adjustmentFactorWhite);
        pieceSquareTables.rook_table[i] = static_cast<int>(pieceSquareTables.rook_table[i] * adjustmentFactorWhite);
        pieceSquareTables.queen_table[i] = static_cast<int>(pieceSquareTables.queen_table[i] * adjustmentFactorWhite);
        pieceSquareTables.king_table_mg[i] = static_cast<int>(pieceSquareTables.king_table_mg[i] * adjustmentFactorWhite);
        pieceSquareTables.king_table_eg[i] = static_cast<int>(pieceSquareTables.king_table_eg[i] * adjustmentFactorWhite);

        // For black, mirror the adjustment
        pieceSquareTables.pawn_table[i] = static_cast<int>(pieceSquareTables.pawn_table[i] * adjustmentFactorBlack);
        pieceSquareTables.knight_table[i] = static_cast<int>(pieceSquareTables.knight_table[i] * adjustmentFactorBlack);
        pieceSquareTables.bishop_table[i] = static_cast<int>(pieceSquareTables.bishop_table[i] * adjustmentFactorBlack);
        pieceSquareTables.rook_table[i] = static_cast<int>(pieceSquareTables.rook_table[i] * adjustmentFactorBlack);
        pieceSquareTables.queen_table[i] = static_cast<int>(pieceSquareTables.queen_table[i] * adjustmentFactorBlack);
        pieceSquareTables.king_table_mg[i] = static_cast<int>(pieceSquareTables.king_table_mg[i] * adjustmentFactorBlack);
        pieceSquareTables.king_table_eg[i] = static_cast<int>(pieceSquareTables.king_table_eg[i] * adjustmentFactorBlack);
    }

    // Optionally, perform further tuning or backtesting iterations
    backtestPieceSquareTables();
}

// Adjust piece-square tables by a factor
void Evaluator::adjustPieceSquareTables(double factor) {
    for (int i = 0; i < 64; ++i) {
        pieceSquareTables.pawn_table[i] = static_cast<int>(pieceSquareTables.pawn_table[i] * factor);
        pieceSquareTables.knight_table[i] = static_cast<int>(pieceSquareTables.knight_table[i] * factor);
        pieceSquareTables.bishop_table[i] = static_cast<int>(pieceSquareTables.bishop_table[i] * factor);
        pieceSquareTables.rook_table[i] = static_cast<int>(pieceSquareTables.rook_table[i] * factor);
        pieceSquareTables.queen_table[i] = static_cast<int>(pieceSquareTables.queen_table[i] * factor);
        pieceSquareTables.king_table_mg[i] = static_cast<int>(pieceSquareTables.king_table_mg[i] * factor);
        pieceSquareTables.king_table_eg[i] = static_cast<int>(pieceSquareTables.king_table_eg[i] * factor);
    }
}

// Backtest piece-square tables to find optimal parameters
void Evaluator::backtestPieceSquareTables() {
    // Store the best configuration found
    int bestScore = -999999;
    double bestFactor = 1.0;

    // Iterate over possible adjustment factors
    for (double factor = 0.9; factor <= 1.1; factor += 0.01) {
        adjustPieceSquareTables(factor);
        int score = simulateGames();

        if (score > bestScore) {
            bestScore = score;
            bestFactor = factor;
        }
    }

    // Set to the best factor found
    adjustPieceSquareTables(bestFactor / 1.0); // Normalize if needed
}

// Handle opening book (simplified example)
Move Evaluator::getOpeningBookMove(const Board& board) const {
    // Load opening book from a file or predefined data
    // For simplicity, return a random move from legal moves
    std::vector<Move> moves = board.generateLegalMoves(WHITE);
    if (!moves.empty()) {
        return moves[0]; // Placeholder: select the first move
    }
    return Move("e2e4"); // Default move if no moves found
}

// Handle endgame tablebases (simplified example)
Move Evaluator::getEndgameTablebaseMove(const Board& board) const {
    // Integrate with Syzygy or other tablebase libraries
    // For simplicity, return a random move from legal moves
    std::vector<Move> moves = board.generateLegalMoves(WHITE);
    if (!moves.empty()) {
        return moves[0]; // Placeholder: select the first move
    }
    return Move("e2e4"); // Default move if no moves found
}

// Determine if the current player is in check (simplified)
bool Evaluator::isCheck(const Board& board, Color color) const {
    // Implement a proper check detection mechanism
    // Placeholder: return false
    return false;
}

