// include/move.h
#pragma once

#include <string>
#include "types.h"

struct Move {
    int       start;   // 0–63
    int       end;     // 0–63
    MoveType  type;    // NORMAL, CAPTURE, PROMOTION, CASTLE_*, EN_PASSANT, INVALID
    char      promo;   // 'Q','R','B','N' or '\0'

    Move()
      : start(-1), end(-1), type(MoveType::INVALID), promo('\0') {}
    Move(int s, int e, MoveType t = MoveType::NORMAL, char p = '\0')
      : start(s), end(e), type(t), promo(p) {}

    bool isValid() const;
    bool isCapture() const;
    std::string toString() const;
};
