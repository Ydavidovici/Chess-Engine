#include "board.h"
#include <array>
#include <cstdlib>  // For random number generation (magic number tuning)

// Example magic numbers for rooks and bishops (precomputed for simplicity)
const uint64_t Board::rookMagicNumbers[64] = {
        0xA180022080400230ULL, 0x0040100040201002ULL, 0x0080088028000000ULL, 0x0080080280800800ULL,
        // Continue with the rest of the 64 squares...
};

const uint64_t Board::bishopMagicNumbers[64] = {
        0x0890042002008001ULL, 0x0002000080804200ULL, 0x0000840800200020ULL, 0x0002001000200800ULL,
        // Continue with the rest of the 64 squares...
};

// Directions for sliding piece movement (Rook: straight, Bishop: diagonals)
const std::vector<int> rookDirections = {1, -1, 8, -8}; // right, left, up, down
const std::vector<int> bishopDirections = {9, -9, 7, -7}; // diagonals

Board::Board() {
    initializeBoard();
    initializeMagicBitboards();
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

// Precompute magic bitboards for rooks, bishops, and queens
void Board::initializeMagicBitboards() {
    for (int square = 0; square < 64; ++square) {
        for (uint64_t blockers = 0; blockers < 4096; ++blockers) {
            rookAttackTable[square][blockers] = generateRookAttacks(square, blockers);
            queenAttackTable[square][blockers] = generateQueenAttacks(square, blockers);
        }
        for (uint64_t blockers = 0; blockers < 512; ++blockers) {
            bishopAttackTable[square][blockers] = generateBishopAttacks(square, blockers);
        }
    }
}

uint64_t Board::generateRookAttacks(int square, uint64_t blockers) const {
    uint64_t attacks = 0ULL;
    for (const int& direction : rookDirections) {
        int currentSquare = square;
        while (true) {
            currentSquare += direction;
            if (currentSquare < 0 || currentSquare >= 64) break;
            attacks |= (1ULL << currentSquare);
            if (blockers & (1ULL << currentSquare)) break; // Stop if blocked
        }
    }
    return attacks;
}

uint64_t Board::generateBishopAttacks(int square, uint64_t blockers) const {
    uint64_t attacks = 0ULL;
    for (const int& direction : bishopDirections) {
        int currentSquare = square;
        while (true) {
            currentSquare += direction;
            if (currentSquare < 0 || currentSquare >= 64) break;
            attacks |= (1ULL << currentSquare);
            if (blockers & (1ULL << currentSquare)) break; // Stop if blocked
        }
    }
    return attacks;
}

uint64_t Board::generateQueenAttacks(int square, uint64_t blockers) const {
    return generateRookAttacks(square, blockers) | generateBishopAttacks(square, blockers);
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

bool Board::makeMove(const std::string& move) {
    int startX = move[1] - '1';
    int startY = move[0] - 'a';
    int endX = move[3] - '1';
    int endY = move[2] - 'a';
    int startPos = getPosition(startX, startY);
    int endPos = getPosition(endX, endY);

    if (isBitSet(whitePawns, startPos)) {
        clearBit(whitePawns, startPos);
        setBit(whitePawns, endPos);
    } else if (isBitSet(whiteKnights, startPos)) {
        clearBit(whiteKnights, startPos);
        setBit(whiteKnights, endPos);
    } else if (isBitSet(whiteBishops, startPos)) {
        clearBit(whiteBishops, startPos);
        setBit(whiteBishops, endPos);
    } else if (isBitSet(whiteRooks, startPos)) {
        clearBit(whiteRooks, startPos);
        setBit(whiteRooks, endPos);
    } else if (isBitSet(whiteQueens, startPos)) {
        clearBit(whiteQueens, startPos);
        setBit(whiteQueens, endPos);
    } else if (isBitSet(whiteKings, startPos)) {
        clearBit(whiteKings, startPos);
        setBit(whiteKings, endPos);
    } else if (isBitSet(blackPawns, startPos)) {
        clearBit(blackPawns, startPos);
        setBit(blackPawns, endPos);
    } else if (isBitSet(blackKnights, startPos)) {
        clearBit(blackKnights, startPos);
        setBit(blackKnights, endPos);
    } else if (isBitSet(blackBishops, startPos)) {
        clearBit(blackBishops, startPos);
        setBit(blackBishops, endPos);
    } else if (isBitSet(blackRooks, startPos)) {
        clearBit(blackRooks, startPos);
        setBit(blackRooks, endPos);
    } else if (isBitSet(blackQueens, startPos)) {
        clearBit(blackQueens, startPos);
        setBit(blackQueens, endPos);
    } else if (isBitSet(blackKings, startPos)) {
        clearBit(blackKings, startPos);
        setBit(blackKings, endPos);
    } else {
        std::cerr << "Invalid move: " << move << std::endl;
        return false;
    }

    return true;
}

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

uint64_t Board::generateKnightMoves(int square) const {
    const uint64_t L1 = 1ULL << (square + 17), L2 = 1ULL << (square + 15);
    const uint64_t R1 = 1ULL << (square - 17), R2 = 1ULL << (square - 15);
    const uint64_t U1 = 1ULL << (square + 10), U2 = 1ULL << (square - 10);
    return L1 | L2 | R1 | R2 | U1 | U2;
}

uint64_t Board::generateKingMoves(int square) const {
    return (1ULL << (square + 1)) | (1ULL << (square - 1)) |
           (1ULL << (square + 8)) | (1ULL << (square - 8)) |
           (1ULL << (square + 9)) | (1ULL << (square - 9)) |
           (1ULL << (square + 7)) | (1ULL << (square - 7));
}
