#pragma once
#include <string>
#include "types.h"

struct Move {
    int start, end;
    MoveType type = MoveType::NORMAL;
    char promo = '\0';  // promotion piece: 'Q','R','B','N' or 0 if none

    // <-- Add this:
    Move() = default;
    Move(int s, int e, MoveType t = MoveType::NORMAL, char p = '\0')
            : start(s), end(e), type(t), promo(p) {}

    bool isValid() const;
    bool isCapture() const;
    std::string toString() const;
};