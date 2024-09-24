#include "board.h"
#include <iostream>

// Constructor
Board::Board() {
    initializeBoard();
}

// Initialize the board to the starting position
void Board::initializeBoard() {
    whitePawns   = 0x000000000000FF00ULL;
    whiteKnights = 0x0000000000000042ULL;
    whiteBishops = 0x0000000000000024ULL;
    whiteRooks   = 0x0000000000000081ULL;
    whiteQueens  = 0x0000000000000008ULL;
    whiteKings   = 0x0000000000000010ULL;

    blackPawns   = 0x00FF000000000000ULL;
    blackKnights = 0x4200000000000000ULL;
    blackBishops = 0x2400000000000000ULL;
    blackRooks   = 0x8100000000000000ULL;
    blackQueens  = 0x0800000000000000ULL;
    blackKings   = 0x1000000000000000ULL;
}

// Get current board state as a string
std::string Board::getBoardState() const {
    std::string state;
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int pos = getPosition(rank, file);
            char piece = '.';
            if (isBitSet(whitePawns, pos)) piece = 'P';
            else if (isBitSet(whiteKnights, pos)) piece = 'N';
            else if (isBitSet(whiteBishops, pos)) piece = 'B';
            else if (isBitSet(whiteRooks, pos)) piece = 'R';
            else if (isBitSet(whiteQueens, pos)) piece = 'Q';
            else if (isBitSet(whiteKings, pos)) piece = 'K';
            else if (isBitSet(blackPawns, pos)) piece = 'p';
            else if (isBitSet(blackKnights, pos)) piece = 'n';
            else if (isBitSet(blackBishops, pos)) piece = 'b';
            else if (isBitSet(blackRooks, pos)) piece = 'r';
            else if (isBitSet(blackQueens, pos)) piece = 'q';
            else if (isBitSet(blackKings, pos)) piece = 'k';
            state += piece;
            state += ' ';
        }
        state += "\n";
    }
    return state;
}

// Make a move on the board
bool Board::makeMove(const Move& move, Color color) {
    if (!move.isValid()) return false;

    int start = move.getStartPosition();
    int end = move.getEndPosition();
    MoveType type = move.getMoveType();

    // Identify the piece being moved
    uint64_t pieceMask = 0;
    if (color == WHITE) {
        if (isBitSet(whitePawns, start)) pieceMask = whitePawns;
        else if (isBitSet(whiteKnights, start)) pieceMask = whiteKnights;
        else if (isBitSet(whiteBishops, start)) pieceMask = whiteBishops;
        else if (isBitSet(whiteRooks, start)) pieceMask = whiteRooks;
        else if (isBitSet(whiteQueens, start)) pieceMask = whiteQueens;
        else if (isBitSet(whiteKings, start)) pieceMask = whiteKings;
    } else {
        if (isBitSet(blackPawns, start)) pieceMask = blackPawns;
        else if (isBitSet(blackKnights, start)) pieceMask = blackKnights;
        else if (isBitSet(blackBishops, start)) pieceMask = blackBishops;
        else if (isBitSet(blackRooks, start)) pieceMask = blackRooks;
        else if (isBitSet(blackQueens, start)) pieceMask = blackQueens;
        else if (isBitSet(blackKings, start)) pieceMask = blackKings;
    }

    if (pieceMask == 0) return false; // No piece found

    // Handle captures
    if (type == CAPTURE || type == EN_PASSANT) {
        if (color == WHITE) {
            if (isBitSet(blackPawns, end)) clearBit(blackPawns, end);
            if (isBitSet(blackKnights, end)) clearBit(blackKnights, end);
            if (isBitSet(blackBishops, end)) clearBit(blackBishops, end);
            if (isBitSet(blackRooks, end)) clearBit(blackRooks, end);
            if (isBitSet(blackQueens, end)) clearBit(blackQueens, end);
            if (isBitSet(blackKings, end)) clearBit(blackKings, end);
        } else {
            if (isBitSet(whitePawns, end)) clearBit(whitePawns, end);
            if (isBitSet(whiteKnights, end)) clearBit(whiteKnights, end);
            if (isBitSet(whiteBishops, end)) clearBit(whiteBishops, end);
            if (isBitSet(whiteRooks, end)) clearBit(whiteRooks, end);
            if (isBitSet(whiteQueens, end)) clearBit(whiteQueens, end);
            if (isBitSet(whiteKings, end)) clearBit(whiteKings, end);
        }
    }

    // Move the piece
    if (color == WHITE) {
        if (isBitSet(whitePawns, start)) {
            clearBit(whitePawns, start);
            setBit(whitePawns, end);
            // Handle promotion
            if (type == PROMOTION && move.getPromotionPiece() != 0) {
                clearBit(whitePawns, end);
                switch (move.getPromotionPiece()) {
                    case 'Q': setBit(whiteQueens, end); break;
                    case 'R': setBit(whiteRooks, end); break;
                    case 'B': setBit(whiteBishops, end); break;
                    case 'N': setBit(whiteKnights, end); break;
                    default: setBit(whiteQueens, end); break;
                }
            }
        }
            // Similar for other white pieces
        else if (isBitSet(whiteKnights, start)) {
            clearBit(whiteKnights, start);
            setBit(whiteKnights, end);
        }
        else if (isBitSet(whiteBishops, start)) {
            clearBit(whiteBishops, start);
            setBit(whiteBishops, end);
        }
        else if (isBitSet(whiteRooks, start)) {
            clearBit(whiteRooks, start);
            setBit(whiteRooks, end);
        }
        else if (isBitSet(whiteQueens, start)) {
            clearBit(whiteQueens, start);
            setBit(whiteQueens, end);
        }
        else if (isBitSet(whiteKings, start)) {
            if (!handleCastling(move, color)) return false;
            clearBit(whiteKings, start);
            setBit(whiteKings, end);
        }
    } else {
        if (isBitSet(blackPawns, start)) {
            clearBit(blackPawns, start);
            setBit(blackPawns, end);
            // Handle promotion
            if (type == PROMOTION && move.getPromotionPiece() != 0) {
                clearBit(blackPawns, end);
                switch (move.getPromotionPiece()) {
                    case 'Q': setBit(blackQueens, end); break;
                    case 'R': setBit(blackRooks, end); break;
                    case 'B': setBit(blackBishops, end); break;
                    case 'N': setBit(blackKnights, end); break;
                    default: setBit(blackQueens, end); break;
                }
            }
        }
            // Similar for other black pieces
        else if (isBitSet(blackKnights, start)) {
            clearBit(blackKnights, start);
            setBit(blackKnights, end);
        }
        else if (isBitSet(blackBishops, start)) {
            clearBit(blackBishops, start);
            setBit(blackBishops, end);
        }
        else if (isBitSet(blackRooks, start)) {
            clearBit(blackRooks, start);
            setBit(blackRooks, end);
        }
        else if (isBitSet(blackQueens, start)) {
            clearBit(blackQueens, start);
            setBit(blackQueens, end);
        }
        else if (isBitSet(blackKings, start)) {
            if (!handleCastling(move, color)) return false;
            clearBit(blackKings, start);
            setBit(blackKings, end);
        }
    }

    // Handle special moves like castling
    return true;
}

bool Board::handleCastling(const Move& move, Color color) {
    if (move.getMoveType() == CASTLE_KINGSIDE) {
        if (color == WHITE) {
            // Move rook from h1 to f1
            if (isBitSet(whiteRooks, 7)) {
                clearBit(whiteRooks, 7);
                setBit(whiteRooks, 5);
                return true;
            }
        } else {
            // Move rook from h8 to f8
            if (isBitSet(blackRooks, 63)) {
                clearBit(blackRooks, 63);
                setBit(blackRooks, 61);
                return true;
            }
        }
    } else if (move.getMoveType() == CASTLE_QUEENSIDE) {
        if (color == WHITE) {
            // Move rook from a1 to d1
            if (isBitSet(whiteRooks, 0)) {
                clearBit(whiteRooks, 0);
                setBit(whiteRooks, 3);
                return true;
            }
        } else {
            // Move rook from a8 to d8
            if (isBitSet(blackRooks, 56)) {
                clearBit(blackRooks, 56);
                setBit(blackRooks, 59);
                return true;
            }
        }
    }
    return false;
}

// Check if a square has any piece
bool Board::isBitSet(uint64_t bitboard, int position) const {
    return (bitboard & (1ULL << position)) != 0;
}

// Set a bit on the bitboard
void Board::setBit(uint64_t& bitboard, int position) {
    bitboard |= (1ULL << position);
}

// Clear a bit on the bitboard
void Board::clearBit(uint64_t& bitboard, int position) {
    bitboard &= ~(1ULL << position);
}

// Convert rank and file to position index
int Board::getPosition(int rank, int file) const {
    return rank * 8 + file;
}

// Generate all legal moves for a given color
std::vector<Move> Board::generateLegalMoves(Color color) const {
    std::vector<Move> legalMoves;
    // Generate moves for all pieces
    // For demonstration, only pawn moves are implemented
    uint64_t pawns = (color == WHITE) ? whitePawns : blackPawns;
    while (pawns) {
        int square = __builtin_ctzll(pawns);
        pawns &= pawns - 1;
        uint64_t pawnMoves = generatePawnMoves(square, color == WHITE);
        for (int target = 0; target < 64; target++) {
            if (pawnMoves & (1ULL << target)) {
                Move move(getPosition(square / 8, square % 8), getPosition(target / 8, target % 8));
                // Determine move type
                if ((color == WHITE && target / 8 == 7) || (color == BLACK && target / 8 == 0)) {
                    move.setMoveType(PROMOTION);
                    move.setPromotionPiece('Q'); // Default to Queen
                } else if ((color == WHITE && (board.getBlackPawns() | board.getBlackKnights() | board.getBlackBishops() |
                                               board.getBlackRooks() | board.getBlackQueens() | board.getBlackKings()) & (1ULL << target)) ||
                           (color == BLACK && (board.getWhitePawns() | board.getWhiteKnights() | board.getWhiteBishops() |
                                               board.getWhiteRooks() | board.getWhiteQueens() | board.getWhiteKings()) & (1ULL << target))) {
                    move.setMoveType(CAPTURE);
                }
                legalMoves.push_back(move);
            }
        }
    }

    // Similarly, generate moves for knights, bishops, rooks, queens, and kings
    // Implement move generation for sliding pieces and special moves

    return legalMoves;
}

// Generate pawn moves (captures and forward moves)
uint64_t Board::generatePawnMoves(int square, bool isWhite) const {
    uint64_t moves = 0ULL;
    if (isWhite) {
        if ((square / 8) < 7) { // Not on the last rank
            // Single forward move
            int target = square + 8;
            if (!(
                    (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKings) |
                    (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKings)
            ) & (1ULL << target)) {
                moves |= (1ULL << target);
                // Double forward move from starting rank
                if (square / 8 == 1) {
                    target += 8;
                    if (!(
                            (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKings) |
                            (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKings)
                    ) & (1ULL << target)) {
                        moves |= (1ULL << target);
                    }
                }
            }

            // Captures
            if (square % 8 != 0) { // Capture to the left
                target = square + 7;
                if (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKings & (1ULL << target)) {
                    moves |= (1ULL << target);
                }
            }
            if (square % 8 != 7) { // Capture to the right
                target = square + 9;
                if (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKings & (1ULL << target)) {
                    moves |= (1ULL << target);
                }
            }

            // En Passant can be implemented here
        }
    } else {
        if ((square / 8) > 0) { // Not on the first rank
            // Single forward move
            int target = square - 8;
            if (!(
                    (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKings) |
                    (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKings)
            ) & (1ULL << target)) {
                moves |= (1ULL << target);
                // Double forward move from starting rank
                if (square / 8 == 6) {
                    target -= 8;
                    if (!(
                            (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKings) |
                            (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKings)
                    ) & (1ULL << target)) {
                        moves |= (1ULL << target);
                    }
                }
            }

            // Captures
            if (square % 8 != 0) { // Capture to the left
                target = square - 9;
                if (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKings & (1ULL << target)) {
                    moves |= (1ULL << target);
                }
            }
            if (square % 8 != 7) { // Capture to the right
                target = square - 7;
                if (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKings & (1ULL << target)) {
                    moves |= (1ULL << target);
                }
            }

            // En Passant can be implemented here
        }
    }
    return moves;
}

// Placeholder for other move generation functions (knights, bishops, etc.)

int Board::getPieceCount() const {
    return __builtin_popcountll(whitePawns) + __builtin_popcountll(whiteKnights) +
           __builtin_popcountll(whiteBishops) + __builtin_popcountll(whiteRooks) +
           __builtin_popcountll(whiteQueens) + __builtin_popcountll(whiteKings) +
           __builtin_popcountll(blackPawns) + __builtin_popcountll(blackKnights) +
           __builtin_popcountll(blackBishops) + __builtin_popcountll(blackRooks) +
           __builtin_popcountll(blackQueens) + __builtin_popcountll(blackKings);
}
