// move.h
#pragma once

#include <string>
#include "types.h"

struct Move {
    int start;               // source square (0–63)
    int end;                 // destination square (0–63)
    MoveType type;           // NORMAL, CAPTURE, PROMOTION, CASTLE_*, EN_PASSANT, etc.
    char promo;              // promotion piece: 'Q','R','B','N' or '\0' if none

    Move()
      : start(-1), end(-1), type(MoveType::INVALID), promo('\0') {}
    Move(int s, int e, MoveType t = MoveType::NORMAL, char p = '\0')
      : start(s), end(e), type(t), promo(p) {}

    bool isValid() const;
    bool isCapture() const;
    std::string toString() const;
};
