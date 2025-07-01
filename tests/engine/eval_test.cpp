// eval_test.cpp
#include <iostream>
#include <cstdlib>
#include "evaluator.h"
#include "board.h"

int main(){
    Evaluator ev;
    Board b;

    struct Test {
        const char* fen;
        Color stm;
        int expected;
        const char* desc;
    } tests[] = {
        // Stalemate → draw = 0
        {"7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", Color::BLACK,  0,        "Stalemate Black"},
        {"7k/5Q2/6K1/8/8/8/8/8 w - - 0 1", Color::WHITE,  0,        "Stalemate White"},
        // Checkmate → -MATE_VALUE for stm
        {"6k1/6Q1/6K1/8/8/8/8/8 b - - 0 1", Color::BLACK, -100000, "Checkmate Black"},
        {"6k1/5ppp/8/8/8/8/5PPP/6K1 w - - 0 1", Color::WHITE,   0,     "No-mate White"}
    };

    for (auto &t : tests) {
        b.loadFEN(t.fen);
        int got = ev.evaluateTerminal(b, t.stm);
        if (got != t.expected) {
            std::cerr << "EvalTest FAILED (" << t.desc << "): got "
                      << got << ", expected " << t.expected << "\n";
            return 1;
        }
    }

    std::cout << "EvalTest all passed\n";
    return 0;
}
