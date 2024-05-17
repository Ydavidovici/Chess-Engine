// backend/engine/src/main.cpp

#include "board.h"

int main() {
    Board board;
    board.initializeBoard();
    board.printBoard();  // Print the board to verify initialization
    return 0;
}
