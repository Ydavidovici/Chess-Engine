// board.cpp
#include "board.h"
#include <sstream>
#include <cctype>

Board::Board() {
    initialize();
}

void Board::initialize() {
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

void Board::loadFEN(const std::string& fen) {
    // Clear all bitboards
    initialize();
    std::stringstream ss(fen);
    std::string rows[8];
    // Extract just the piece placement section
    ss >> rows[0];
    for(int i = 1; i < 8; ++i) {
        std::getline(ss, rows[i], '/');
    }
    for(int rank = 7; rank >= 0; --rank) {
        const std::string& row = rows[7 - rank];
        int file = 0;
        for(char c : row) {
            if (std::isdigit(c)) {
                file += c - '0';
            } else {
                int sq = rank * 8 + file;
                switch(c) {
                    case 'P': set(whitePawns,   sq); break;
                    case 'N': set(whiteKnights, sq); break;
                    case 'B': set(whiteBishops, sq); break;
                    case 'R': set(whiteRooks,   sq); break;
                    case 'Q': set(whiteQueens,  sq); break;
                    case 'K': set(whiteKings,   sq); break;
                    case 'p': set(blackPawns,   sq); break;
                    case 'n': set(blackKnights, sq); break;
                    case 'b': set(blackBishops, sq); break;
                    case 'r': set(blackRooks,   sq); break;
                    case 'q': set(blackQueens,  sq); break;
                    case 'k': set(blackKings,   sq); break;
                }
                ++file;
            }
        }
    }
}

std::vector<std::string> Board::getBoardState() const {
    std::vector<std::string> board;
    board.reserve(8);
    for(int rank = 7; rank >= 0; --rank) {
        std::string line;
        line.reserve(16);
        for(int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            char p = '.';
            if      (bit(whitePawns,   sq)) p = 'P';
            else if (bit(whiteKnights, sq)) p = 'N';
            else if (bit(whiteBishops, sq)) p = 'B';
            else if (bit(whiteRooks,   sq)) p = 'R';
            else if (bit(whiteQueens,  sq)) p = 'Q';
            else if (bit(whiteKings,   sq)) p = 'K';
            else if (bit(blackPawns,   sq)) p = 'p';
            else if (bit(blackKnights, sq)) p = 'n';
            else if (bit(blackBishops, sq)) p = 'b';
            else if (bit(blackRooks,   sq)) p = 'r';
            else if (bit(blackQueens,  sq)) p = 'q';
            else if (bit(blackKings,   sq)) p = 'k';
            line.push_back(p);
            line.push_back(' ');
        }
        board.push_back(line);
    }
    return board;
}

bool Board::makeMove(const Move& m, Color c) {
    // TODO: port your Python make_move logic here:
    // 1) remove captured piece
    // 2) handle promotions
    // 3) move the piece bit
    return false;
}

std::vector<Move> Board::generateLegalMoves(Color c) const {
    std::vector<Move> moves;
    // TODO: port pawn, knight, sliding and king move generation
    return moves;
}