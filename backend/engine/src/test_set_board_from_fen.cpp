#include "board.h"
#include <iostream>

int main() {
    Board board;
    board.initializeBoard();
    board.printBoard();

    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (board.setBoardFromFEN(fen)) {
        std::cout << "FEN set successfully!" << std::endl;
        board.printBoard();
    } else {
        std::cerr << "Error setting FEN!" << std::endl;
    }

    return 0;
}
