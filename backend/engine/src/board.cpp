#include "board.h"
#include <iostream>

Board::Board() {
    board.resize(8, std::vector<Piece*>(8, nullptr));
}

void Board::initializeBoard() {
    // Initialize pieces for both sides
    for (int i = 0; i < 8; ++i) {
        board[1][i] = new Piece(Piece::PAWN, Piece::WHITE);
        board[6][i] = new Piece(Piece::PAWN, Piece::BLACK);
    }
    // Other pieces initialization
    board[0][0] = board[0][7] = new Piece(Piece::ROOK, Piece::WHITE);
    board[0][1] = board[0][6] = new Piece(Piece::KNIGHT, Piece::WHITE);
    board[0][2] = board[0][5] = new Piece(Piece::BISHOP, Piece::WHITE);
    board[0][3] = new Piece(Piece::QUEEN, Piece::WHITE);
    board[0][4] = new Piece(Piece::KING, Piece::WHITE);

    board[7][0] = board[7][7] = new Piece(Piece::ROOK, Piece::BLACK);
    board[7][1] = board[7][6] = new Piece(Piece::KNIGHT, Piece::BLACK);
    board[7][2] = board[7][5] = new Piece(Piece::BISHOP, Piece::BLACK);
    board[7][3] = new Piece(Piece::QUEEN, Piece::BLACK);
    board[7][4] = new Piece(Piece::KING, Piece::BLACK);
}

void Board::printBoard() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != nullptr) {
                std::cout << board[i][j]->getSymbol() << " ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
}

std::string Board::getBoardState() const {
    // Implement the method to return the board state as a string
    std::string state;
    for (const auto& row : board) {
        for (const auto& piece : row) {
            state += (piece ? piece->getSymbol() : '.');
            state += ' ';
        }
        state += '\n';
    }
    return state;
}
