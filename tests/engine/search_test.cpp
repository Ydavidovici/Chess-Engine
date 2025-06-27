// search_test.cpp
#include <iostream>
#include <cstdlib>
#include "search.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "board.h"
#include "move.h"

int main(){
    Evaluator ev;
    TranspositionTable tt(1 << 16);
    Search s(ev, tt);
    Board b;

    struct Test {
        const char* fen;
        const char* expectedUci;
        int         depth;
        const char* desc;
    } tests[] = {
        // Mate in 1: Qh5-e8#
        {
            "rnb1kbnr/pppp1ppp/8/4p2Q/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1",
            "h5e8", 1, "Mate in one"
          },
          // Simple capture: Ra1xa8
          {
              "r7/8/8/8/8/8/8/R3K2R w KQ - 0 1",
              "a1a8", 1, "Simple rook capture"
            }
    };

    for (auto &t : tests) {
        b.loadFEN(t.fen);
        Move best = s.findBestMove(b, b.sideToMove(), t.depth);
        std::string u = best.toString();
        if (u != t.expectedUci) {
            std::cerr << "SearchTest FAILED (" << t.desc << "): got "
                      << u << ", expected " << t.expectedUci << "\n";
            return 1;
        }
    }

    std::cout << "SearchTest all passed\n";
    return 0;
}
