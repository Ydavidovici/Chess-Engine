#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "board.h"
#include "evaluator.h"
#include "move.h"
#include "player.h"

class Engine {
public:
    Engine();
    void initialize();
    std::string getBoardState() const;
    bool makeMove(const Move& move);
    int evaluateBoard() const;

private:
    Board board;
    Evaluator evaluator;
    Player whitePlayer; // Player for the white pieces
    Player blackPlayer; // Player for the black pieces
};

#endif // ENGINE_H
