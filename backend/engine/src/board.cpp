#include "board.h"
#include <iostream>

Board::Board() {
    initializeBoard();
}

void Board::initializeBoard() {
    whitePawns = 0x000000000000FF00ULL;
    whiteKnights = 0x0000000000000042ULL;
    whiteBishops = 0x0000000000000024ULL;
    whiteRooks = 0x0000000000000081ULL;
    whiteQueens = 0x0000000000000008ULL;
    whiteKings = 0x0000000000000010ULL;

    blackPawns = 0x00FF000000000000ULL;
    blackKnights = 0x4200000000000000ULL;
    blackBishops = 0x2400000000000000ULL;
    blackRooks = 0x8100000000000000ULL;
    blackQueens = 0x0800000000000000ULL;
    blackKings = 0x1000000000000000ULL;
}

// Generate pawn moves for white and black
uint64_t Board::generatePawnMoves(int square, bool isWhite) const {
    uint64_t moves = 0ULL;
    if (isWhite) {
        if (!(square >= 56 && square < 64)) {
            moves |= (1ULL << (square + 8)); // Single forward move
            if (square >= 8 && square < 16) {
                moves |= (1ULL << (square + 16)); // Double forward move from starting rank
            }
        }
    } else {
        if (!(square >= 0 && square < 8)) {
            moves |= (1ULL << (square - 8)); // Single forward move
            if (square >= 48 && square < 56) {
                moves |= (1ULL << (square - 16)); // Double forward move from starting rank
            }
        }
    }
    return moves;
}

// Generate knight moves
uint64_t Board::generateKnightMoves(int square) const {
    const uint64_t L1 = 1ULL << (square + 17), L2 = 1ULL << (square + 15);
    const uint64_t R1 = 1ULL << (square - 17), R2 = 1ULL << (square - 15);
    const uint64_t U1 = 1ULL << (square + 10), U2 = 1ULL << (square - 10);
    return L1 | L2 | R1 | R2 | U1 | U2;
}

// Generate king moves
uint64_t Board::generateKingMoves(int square) const {
    return (1ULL << (square + 1)) | (1ULL << (square - 1)) |
           (1ULL << (square + 8)) | (1ULL << (square - 8)) |
           (1ULL << (square + 9)) | (1ULL << (square - 9)) |
           (1ULL << (square + 7)) | (1ULL << (square - 7));
}

bool Board::isBitSet(uint64_t bitboard, int position) const {
    return bitboard & (1ULL << position);
}

void Board::setBit(uint64_t& bitboard, int position) {
    bitboard |= (1ULL << position);
}

void Board::clearBit(uint64_t& bitboard, int position) {
    bitboard &= ~(1ULL << position);
}

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
        state += '\n';
    }
    return state;
}

bool Board::makeMove(const Move& move) {
    int start = move.getStartPosition();
    int end = move.getEndPosition();
    // Add logic for all piece movement on the board using bit manipulation.
    // Currently, we’ll just print the move and pretend the move was made.
    std::cout << "Moving from " << move.getMove() << std::endl;
    return true;
}

int Board::getPosition(int rank, int file) const {
    return rank * 8 + file;
}
