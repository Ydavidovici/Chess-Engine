// backend/engine/include/move.h

#ifndef MOVE_H
#define MOVE_H

#include <string>

class Move {
public:
    Move(const std::string& move);
    std::string getMove() const;

private:
    std::string move;
};

#endif // MOVE_H
