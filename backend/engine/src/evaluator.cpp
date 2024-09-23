#include "evaluator.h"
#include <bitset>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>

// Initialize piece-square tables with standard values
class PieceSquareTables {
public:
    int pawn_table[64];
    int knight_table[64];
    int bishop_table[64];
    int rook_table[64];
    int queen_table[64];
    int king_table_mg[64];  // Middlegame King table
    int king_table_eg[64];  // Endgame King table

    int pawn_table_black[64];
    int knight_table_black[64];
    int bishop_table_black[64];
    int rook_table_black[64];
    int queen_table_black[64];
    int king_table_black_mg[64];
    int king_table_black_eg[64];

    void initialize();
};

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

    // Black Pawn Table (inverted)
    for (int i = 0; i < 64; ++i) {
        pawn_table_black[i] = pawn_table[63 - i];
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

    // Black Knight Table (inverted)
    for (int i = 0; i < 64; ++i) {
        knight_table_black[i] = knight_table[63 - i];
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

    // Black Bishop Table (inverted)
    for (int i = 0; i < 64; ++i) {
        bishop_table_black[i] = bishop_table[63 - i];
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

    // Black Rook Table (inverted)
    for (int i = 0; i < 64; ++i) {
        rook_table_black[i] = rook_table[63 - i];
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

    // Black Queen Table (inverted)
    for (int i = 0; i < 64; ++i) {
        queen_table_black[i] = queen_table[63 - i];
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

    // Black King Table (Middlegame, inverted)
    for (int i = 0; i < 64; ++i) {
        king_table_black_mg[i] = king_table_mg[63 - i];
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

    // Black King Table (Endgame, inverted)
    for (int i = 0; i < 64; ++i) {
        king_table_black_eg[i] = king_table_eg[63 - i];
    }
}


// Initialize Evaluator
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
    material += PAWN_VALUE * countBits(board.getWhitePawns());
    material += KNIGHT_VALUE * countBits(board.getWhiteKnights());
    material += BISHOP_VALUE * countBits(board.getWhiteBishops());
    material += ROOK_VALUE * countBits(board.getWhiteRooks());
    material += QUEEN_VALUE * countBits(board.getWhiteQueens());
    material += 20000 * countBits(board.getWhiteKings());

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

    // Evaluate white pieces
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

    // Evaluate black pieces
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

// Tapered Evaluation between middlegame and endgame
int Evaluator::taperedEval(const Board& board, int phase) const {
    int mgScore = evaluateMiddlegame(board);
    int egScore = evaluateEndgame(board);
    int taperedScore = ((mgScore * phase) + (egScore * (24 - phase))) / 24;  // Linear interpolation
    return taperedScore;
}

// Evaluate King Safety (simplified pawn shield)
int Evaluator::evaluateKingSafety(const Board& board) const {
    int score = 0;

    uint64_t whiteKing = board.getWhiteKings();
    uint64_t blackKing = board.getBlackKings();

    // Example King Safety evaluation logic
    // In practice, use more complex methods like pawn shielding, piece proximity, etc.
    uint64_t whiteKingZone = (whiteKing << 1) | (whiteKing >> 1);
    uint64_t blackKingZone = (blackKing << 1) | (blackKing >> 1);

    score += countBits(whiteKingZone & board.getWhitePawns()) * 20;
    score -= countBits(blackKingZone & board.getBlackPawns()) * 20;

    return score;
}

// Evaluate passed pawns
int Evaluator::evaluatePassedPawns(const Board& board) const {
    int score = 0;

    // Evaluate white passed pawns
    uint64_t whitePawns = board.getWhitePawns();
    while (whitePawns) {
        int square = __builtin_ctzll(whitePawns);  // Get index of first set bit
        whitePawns &= whitePawns - 1;  // Clear least significant bit
        if (!(board.getBlackPawns() & (1ULL << square))) {
            score += 50;  // Bonus for white passed pawns
        }
    }

    // Evaluate black passed pawns
    uint64_t blackPawns = board.getBlackPawns();
    while (blackPawns) {
        int square = __builtin_ctzll(blackPawns);  // Get index of first set bit
        blackPawns &= blackPawns - 1;
        if (!(board.getWhitePawns() & (1ULL << square))) {
            score -= 50;  // Bonus for black passed pawns
        }
    }

    return score;
}

// Advanced Pawn Structure Evaluation
int Evaluator::evaluatePawnStructure(const Board& board) const {
    int score = 0;

    // Penalize isolated, doubled, and backward pawns
    uint64_t whitePawns = board.getWhitePawns();
    uint64_t blackPawns = board.getBlackPawns();

    // Example of simple pawn structure penalties
    // Penalize isolated pawns
    score -= countBits(whitePawns & ~(whitePawns >> 1)) * 20;  // White isolated pawns
    score += countBits(blackPawns & ~(blackPawns << 1)) * 20;  // Black isolated pawns

    return score;
}

// Bishop Pair Bonus
int Evaluator::evaluateBishopPair(const Board& board) const {
    int score = 0;
    if (countBits(board.getWhiteBishops()) == 2) score += 50;  // Bonus for white bishop pair
    if (countBits(board.getBlackBishops()) == 2) score -= 50;  // Bonus for black bishop pair
    return score;
}

// Rook on Open File Bonus
int Evaluator::evaluateRookOpenFile(const Board& board) const {
    int score = 0;

    // Reward rooks placed on open or semi-open files
    uint64_t whiteRooks = board.getWhiteRooks();
    uint64_t blackRooks = board.getBlackRooks();

    for (int square = 0; square < 64; ++square) {
        if (whiteRooks & (1ULL << square)) {
            // Check if the file is open or semi-open
            uint64_t fileMask = 0x0101010101010101ULL << (square % 8);
            if (!(board.getBlackPawns() & fileMask)) {
                score += 40;  // Bonus for rooks on open files
            }
        }

        if (blackRooks & (1ULL << square)) {
            uint64_t fileMask = 0x0101010101010101ULL << (square % 8);
            if (!(board.getWhitePawns() & fileMask)) {
                score -= 40;  // Bonus for black rooks on open files
            }
        }
    }

    return score;
}

// Evaluate mobility
int Evaluator::evaluateMobility(const Board& board, Color color) const {
    return generateLegalMoves(board, color).size();
}

// Evaluate threats to opponent's pieces
int Evaluator::evaluateThreats(const Board& board, Color color) const {
    int score = 0;

    // Example: Reward for threatening the opponent's high-value pieces
    uint64_t opponentPieces = color == Color::WHITE ? board.getBlackQueens() | board.getBlackRooks() : board.getWhiteQueens() | board.getWhiteRooks();

    std::vector<Move> moves = generateLegalMoves(board, color);
    for (const auto& move : moves) {
        if (opponentPieces & (1ULL << move.getTargetSquare())) {
            score += 30;  // Reward for threatening high-value opponent pieces
        }
    }

    return score;
}

// Full evaluation function combining all components
int Evaluator::evaluate(const Board& board) const {
    int score = 0;
    int phase = calculateGamePhase(board);  // Calculate game phase (opening, middlegame, endgame)

    // Apply all evaluation factors
    score += evaluateMaterial(board);
    score += evaluatePieceSquare(board);
    score += evaluateKingSafety(board);
    score += evaluatePassedPawns(board);
    score += evaluateMobility(board, Color::WHITE) - evaluateMobility(board, Color::BLACK);
    score += evaluateThreats(board, Color::WHITE) - evaluateThreats(board, Color::BLACK);
    score += evaluatePawnStructure(board);
    score += evaluateBishopPair(board);
    score += evaluateRookOpenFile(board);

    // Use tapered evaluation based on game phase
    return taperedEval(board, phase);
}

// Quiescence Search for tactical evaluation
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

// Iterative Deepening
int Evaluator::iterativeDeepening(Board& board, int maxDepth, bool maximizingPlayer) {
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();
    std::unordered_map<uint64_t, HashEntry> transpositionTable;
    int bestEval = 0;
    for (int depth = 1; depth <= maxDepth; ++depth) {
        bestEval = search(board, depth, alpha, beta, maximizingPlayer, transpositionTable);
    }
    return bestEval;
}

// Principal Variation Search (PVS)
int Evaluator::search(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable) {
    if (depth == 0) return quiescenceSearch(board, alpha, beta, maximizingPlayer);
    std::vector<Move> moves = generateLegalMoves(board, maximizingPlayer ? Color::WHITE : Color::BLACK);
    moveOrdering(moves, board, maximizingPlayer);
    bool pvNode = true;
    for (const auto& move : moves) {
        Board newBoard = board;
        newBoard.makeMove(move, maximizingPlayer ? Color::WHITE : Color::BLACK);
        int score;
        if (pvNode) {
            score = -search(newBoard, depth - 1, -beta, -alpha, !maximizingPlayer, transpositionTable);
            pvNode = false;
        } else {
            score = -search(newBoard, depth - 1, -alpha - 1, -alpha, !maximizingPlayer, transpositionTable);
            if (score > alpha && score < beta) score = -search(newBoard, depth - 1, -beta, -alpha, !maximizingPlayer, transpositionTable);
        }
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// Null Move Pruning
int Evaluator::nullMovePruning(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, std::unordered_map<uint64_t, HashEntry>& transpositionTable) {
    if (maximizingPlayer && depth > 2) {
        Board newBoard = board;
        int eval = -minimax(newBoard, depth - 2, -beta, -alpha, !maximizingPlayer, transpositionTable);
        if (eval >= beta) return beta;
    }
    return minimax(board, depth, alpha, beta, maximizingPlayer, transpositionTable);
}

// Transposition table storage
void Evaluator::storeTransposition(std::unordered_map<uint64_t, HashEntry>& transpositionTable, uint64_t hash, int score, int depth, int flag, int type) {
    HashEntry entry = { hash, depth, score, flag, type };
    transpositionTable[hash] = entry;
}

// Integrate Stockfish for comparison
void Evaluator::integrateStockfish(const Board& board) const {
    FILE* stockfish = popen("stockfish", "w");
    if (!stockfish) {
        std::cerr << "Failed to open Stockfish!" << std::endl;
        return;
    }
    fprintf(stockfish, "uci\n");
    std::string fen = board.toFEN();
    std::string positionCommand = "position fen " + fen + "\n";
    fprintf(stockfish, "%s", positionCommand.c_str());
    fprintf(stockfish, "go depth 20\n");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), stockfish)) {
        std::string response = buffer;
        if (response.find("bestmove") != std::string::npos) break;
        if (response.find("info depth") != std::string::npos) {
            std::cout << "Stockfish evaluation: " << response << std::endl;
        }
    }
    pclose(stockfish);
}

// Opening book support
Move Evaluator::getOpeningBookMove(const Board& board) const {
    std::ifstream bookFile("opening_book.txt");
    if (!bookFile.is_open()) {
        std::cerr << "Failed to open the opening book!" << std::endl;
        return Move();
    }
    std::string fen = board.toFEN();
    std::string line;
    while (std::getline(bookFile, line)) {
        size_t split = line.find(' ');
        std::string bookFen = line.substr(0, split);
        std::string move = line.substr(split + 1);
        if (bookFen == fen) {
            bookFile.close();
            return Move(move);
        }
    }
    bookFile.close();
    return Move();
}

// Endgame tablebase support
Move Evaluator::getEndgameTablebaseMove(const Board& board) const {
    if (board.getPieceCount() <= 6) {
        std::string fen = board.toFEN();
        TablebaseResult result = syzygy_probe_wdl(fen.c_str());
        if (result.is_valid) return Move(result.best_move);
    }
    return Move();
}

// Parameter tuning based on game results
void Evaluator::tuneParameters(const GameResult& result) {
    if (result.win) adjustPieceSquareTables(1);
    else if (result.loss) adjustPieceSquareTables(-1);
    else if (result.draw) adjustPieceSquareTables(0.1);
}

void Evaluator::adjustPieceSquareTables(double factor) {
    // Adjust the tables for both white and black pieces, for all piece types

    for (int i = 0; i < 64; ++i) {
        // Adjust all piece-square tables for white
        pieceSquareTables.pawn_table[i] += static_cast<int>(factor * pieceSquareTables.pawn_table[i]);
        pieceSquareTables.knight_table[i] += static_cast<int>(factor * pieceSquareTables.knight_table[i]);
        pieceSquareTables.bishop_table[i] += static_cast<int>(factor * pieceSquareTables.bishop_table[i]);
        pieceSquareTables.rook_table[i] += static_cast<int>(factor * pieceSquareTables.rook_table[i]);
        pieceSquareTables.queen_table[i] += static_cast<int>(factor * pieceSquareTables.queen_table[i]);
        pieceSquareTables.king_table_mg[i] += static_cast<int>(factor * pieceSquareTables.king_table_mg[i]);
        pieceSquareTables.king_table_eg[i] += static_cast<int>(factor * pieceSquareTables.king_table_eg[i]);

        // Adjust all piece-square tables for black
        pieceSquareTables.pawn_table_black[i] += static_cast<int>(factor * pieceSquareTables.pawn_table_black[i]);
        pieceSquareTables.knight_table_black[i] += static_cast<int>(factor * pieceSquareTables.knight_table_black[i]);
        pieceSquareTables.bishop_table_black[i] += static_cast<int>(factor * pieceSquareTables.bishop_table_black[i]);
        pieceSquareTables.rook_table_black[i] += static_cast<int>(factor * pieceSquareTables.rook_table_black[i]);
        pieceSquareTables.queen_table_black[i] += static_cast<int>(factor * pieceSquareTables.queen_table_black[i]);
        pieceSquareTables.king_table_black_mg[i] += static_cast<int>(factor * pieceSquareTables.king_table_black_mg[i]);
        pieceSquareTables.king_table_black_eg[i] += static_cast<int>(factor * pieceSquareTables.king_table_black_eg[i]);
    }
}

void Evaluator::backtestPieceSquareTables() {
    // Best table configurations
    int bestPawnTable[64], bestKnightTable[64], bestBishopTable[64], bestRookTable[64], bestQueenTable[64], bestKingTableMg[64], bestKingTableEg[64];
    int bestPawnTableBlack[64], bestKnightTableBlack[64], bestBishopTableBlack[64], bestRookTableBlack[64], bestQueenTableBlack[64], bestKingTableBlackMg[64], bestKingTableBlackEg[64];

    std::copy(std::begin(pieceSquareTables.pawn_table), std::end(pieceSquareTables.pawn_table), bestPawnTable);
    std::copy(std::begin(pieceSquareTables.knight_table), std::end(pieceSquareTables.knight_table), bestKnightTable);
    std::copy(std::begin(pieceSquareTables.bishop_table), std::end(pieceSquareTables.bishop_table), bestBishopTable);
    std::copy(std::begin(pieceSquareTables.rook_table), std::end(pieceSquareTables.rook_table), bestRookTable);
    std::copy(std::begin(pieceSquareTables.queen_table), std::end(pieceSquareTables.queen_table), bestQueenTable);
    std::copy(std::begin(pieceSquareTables.king_table_mg), std::end(pieceSquareTables.king_table_mg), bestKingTableMg);
    std::copy(std::begin(pieceSquareTables.king_table_eg), std::end(pieceSquareTables.king_table_eg), bestKingTableEg);

    std::copy(std::begin(pieceSquareTables.pawn_table_black), std::end(pieceSquareTables.pawn_table_black), bestPawnTableBlack);
    std::copy(std::begin(pieceSquareTables.knight_table_black), std::end(pieceSquareTables.knight_table_black), bestKnightTableBlack);
    std::copy(std::begin(pieceSquareTables.bishop_table_black), std::end(pieceSquareTables.bishop_table_black), bestBishopTableBlack);
    std::copy(std::begin(pieceSquareTables.rook_table_black), std::end(pieceSquareTables.rook_table_black), bestRookTableBlack);
    std::copy(std::begin(pieceSquareTables.queen_table_black), std::end(pieceSquareTables.queen_table_black), bestQueenTableBlack);
    std::copy(std::begin(pieceSquareTables.king_table_black_mg), std::end(pieceSquareTables.king_table_black_mg), bestKingTableBlackMg);
    std::copy(std::begin(pieceSquareTables.king_table_black_eg), std::end(pieceSquareTables.king_table_black_eg), bestKingTableBlackEg);

    int bestScore = -999999;

    for (double adjustmentFactor = 0.9; adjustmentFactor <= 1.1; adjustmentFactor += 0.01) {
        adjustPieceSquareTables(adjustmentFactor);
        int score = simulateGames();

        if (score > bestScore) {
            bestScore = score;
            std::copy(std::begin(pieceSquareTables.pawn_table), std::end(pieceSquareTables.pawn_table), bestPawnTable);
            std::copy(std::begin(pieceSquareTables.knight_table), std::end(pieceSquareTables.knight_table), bestKnightTable);
            std::copy(std::begin(pieceSquareTables.bishop_table), std::end(pieceSquareTables.bishop_table), bestBishopTable);
            std::copy(std::begin(pieceSquareTables.rook_table), std::end(pieceSquareTables.rook_table), bestRookTable);
            std::copy(std::begin(pieceSquareTables.queen_table), std::end(pieceSquareTables.queen_table), bestQueenTable);
            std::copy(std::begin(pieceSquareTables.king_table_mg), std::end(pieceSquareTables.king_table_mg), bestKingTableMg);
            std::copy(std::begin(pieceSquareTables.king_table_eg), std::end(pieceSquareTables.king_table_eg), bestKingTableEg);

            std::copy(std::begin(pieceSquareTables.pawn_table_black), std::end(pieceSquareTables.pawn_table_black), bestPawnTableBlack);
            std::copy(std::begin(pieceSquareTables.knight_table_black), std::end(pieceSquareTables.knight_table_black), bestKnightTableBlack);
            std::copy(std::begin(pieceSquareTables.bishop_table_black), std::end(pieceSquareTables.bishop_table_black), bestBishopTableBlack);
            std::copy(std::begin(pieceSquareTables.rook_table_black), std::end(pieceSquareTables.rook_table_black), bestRookTableBlack);
            std::copy(std::begin(pieceSquareTables.queen_table_black), std::end(pieceSquareTables.queen_table_black), bestQueenTableBlack);
            std::copy(std::begin(pieceSquareTables.king_table_black_mg), std::end(pieceSquareTables.king_table_black_mg), bestKingTableBlackMg);
            std::copy(std::begin(pieceSquareTables.king_table_black_eg), std::end(pieceSquareTables.king_table_black_eg), bestKingTableBlackEg);
        }
    }

    std::copy(std::begin(bestPawnTable), std::end(bestPawnTable), pieceSquareTables.pawn_table);
    std::copy(std::begin(bestKnightTable), std::end(bestKnightTable), pieceSquareTables.knight_table);
    std::copy(std::begin(bestBishopTable), std::end(bestBishopTable), pieceSquareTables.bishop_table);
    std::copy(std::begin(bestRookTable), std::end(bestRookTable), pieceSquareTables.rook_table);
    std::copy(std::begin(bestQueenTable), std::end(bestQueenTable), pieceSquareTables.queen_table);
    std::copy(std::begin(bestKingTableMg), std::end(bestKingTableMg), pieceSquareTables.king_table_mg);
    std::copy(std::begin(bestKingTableEg), std::end(bestKingTableEg), pieceSquareTables.king_table_eg);

    std::copy(std::begin(bestPawnTableBlack), std::end(bestPawnTableBlack), pieceSquareTables.pawn_table_black);
    std::copy(std::begin(bestKnightTableBlack), std::end(bestKnightTableBlack), pieceSquareTables.knight_table_black);
    std::copy(std::begin(bestBishopTableBlack), std::end(bestBishopTableBlack), pieceSquareTables.bishop_table_black);
    std::copy(std::begin(bestRookTableBlack), std::end(bestRookTableBlack), pieceSquareTables.rook_table_black);
    std::copy(std::begin(bestQueenTableBlack), std::end(bestQueenTableBlack), pieceSquareTables.queen_table_black);
    std::copy(std::begin(bestKingTableBlackMg), std::end(bestKingTableBlackMg), pieceSquareTables.king_table_black_mg);
    std::copy(std::begin(bestKingTableBlackEg), std::end(bestKingTableBlackEg), pieceSquareTables.king_table_black_eg);
}

// Generate all legal moves for a given color
std::vector<Move> Evaluator::generateLegalMoves(const Board& board, Color color) const {
    std::vector<Move> legalMoves;
    // Add logic to generate all legal moves for the specified color
    return legalMoves;
}
