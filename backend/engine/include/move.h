#ifndef MOVE_H
#define MOVE_H

#include <string>

class Move {
public:
    Move(const std::string& move);

    // Get start and end positions on the board (0 to 63)
    int getStartPosition() const;
    int getEndPosition() const;

    // New getters to replace missing functions in the error log
    std::string getMove() const;
    int getStartRow() const;
    int getStartCol() const;
    int getEndRow() const;
    int getEndCol() const;

private:
    int startPosition;
    int endPosition;
};

#endif // MOVE_H
