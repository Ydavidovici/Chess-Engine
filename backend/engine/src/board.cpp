#include "board.h"

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

void Board::printBoard() const {
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
            std::cout << piece << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
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

    std::cout << "Making move from " << startPos << " to " << endPos << std::endl;

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

bool Board::isBitSet(uint64_t bitboard, int position) const {
    return bitboard & (1ULL << position);
}

void Board::setBit(uint64_t& bitboard, int position) {
    bitboard |= (1ULL << position);
}

void Board::clearBit(uint64_t& bitboard, int position) {
    bitboard &= ~(1ULL << position);
}
