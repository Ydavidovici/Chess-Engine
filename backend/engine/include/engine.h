#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "board.h"
#include "evaluator.h"
#include "player.h"

class Engine {
public:
    Engine();
    void initialize();
    bool makeMove(const std::string& move);
    std::string getBoardState() const;
    int evaluateBoard() const;

private:
    Board board;
    Evaluator evaluator;
    Player whitePlayer;
    Player blackPlayer;
};

#endif // ENGINE_H
