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

    // ——————————————————————————————————————————
    // Added so tests can do Move::fromUCI(...)
    bool operator==(const Move& o) const {
        return start == o.start
            && end   == o.end
            && type  == o.type
            && promo == o.promo;
    }

    static Move fromUCI(const std::string& uci) {
        // must be at least "e2e4"
        if (uci.size() < 4) return Move();
        int f0 = uci[0] - 'a';
        int r0 = uci[1] - '1';
        int f1 = uci[2] - 'a';
        int r1 = uci[3] - '1';
        if (f0<0||f0>7||r0<0||r0>7||f1<0||f1>7||r1<0||r1>7)
            return Move();
        int s = r0*8 + f0;
        int e = r1*8 + f1;
        // promotion?
        if (uci.size() == 5) {
            char p = std::toupper(uci[4]);
            return Move(s, e, MoveType::PROMOTION, p);
        }
        return Move(s, e);
    }
};
