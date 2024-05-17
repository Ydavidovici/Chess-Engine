// backend/engine/include/board.h

#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "piece.h"

class Board {
public:
    Board();
    void initializeBoard();
    void printBoard();  // For debugging purposes

private:
    std::vector<std::vector<Piece*>> board;  // 8x8 board
};

#endif // BOARD_H
