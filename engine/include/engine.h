// engine/include/engine.h
#pragma once

#include "board.h"
#include "move.h"
#include "types.h"

class Engine {
public:
    Engine();
    void printBoard() const;
    bool makeMove(const Move& m, Color c);
    Move getLastMove() const;

private:
    Board board;
    Move lastMove;
};