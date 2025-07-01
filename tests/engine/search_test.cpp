#include <iostream>
#include <cstdlib>
#include <limits>
#include <vector>
#include <string>
#include <algorithm>

#include "search.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "board.h"
#include "move.h"
#include "timeman.h"

int main(){
    Evaluator ev;
    TranspositionTable tt(1 << 16);
    Search s(ev, tt);
    Board b;

    // --- Mate-in-one test (drop-in) ---
    {
        const char* fen = "1k4N1/6Q1/1K6/8/8/8/8/8 w - - 0 1";
        std::cout << "=== Mate-in-One Test ===\n";
        b.loadFEN(fen);

        // brute-force: find all true mates-in-one
        auto legal = b.generateLegalMoves();
        std::vector<std::string> mates;
        for (auto &m : legal) {
            if (!b.makeMove(m)) continue;
            bool isMate = b.isCheckmate(b.sideToMove() == Color::WHITE
                                        ? Color::BLACK
                                        : Color::WHITE);
            b.unmakeMove();
            if (isMate) mates.push_back(m.toString());
        }
        std::cout << "Brute-force mate-in-one moves: ";
        if (mates.empty()) {
            std::cout << "(none)\n";
        } else {
            for (auto &u : mates) std::cout << u << " ";
            std::cout << "\n";
        }

        // engine: best at depth=1
        TimeManager tmMate;
        tmMate.start(1'000'000, 0, 1);
        Move bestMate = s.findBestMove(b, b.sideToMove(), 1, tmMate);
        std::cout << "Engine best (depth=1): " << bestMate.toString() << "\n\n";
    }

    // --- Simple capture tests under infinite time budget ---
    struct CaptureTest {
        const char* fen;
        std::vector<std::string> expectedUcis;
        const char* desc;
    } captureTests[] = {
        { "r6k/8/8/8/8/8/8/R3K3 w Q - 0 1",
          {"a1a8"}, "Simple rook capture"
        }
    };

    for (auto &t : captureTests) {
        b.loadFEN(t.fen);
        TimeManager tm;
        // give ample time so search actually runs depth=1
        tm.start(1'000'000, /*inc=*/0, /*moves_to_go=*/1);
        Move best = s.findBestMove(b, b.sideToMove(), 1, tm);
        std::string u = best.toString();
        bool ok = std::find(t.expectedUcis.begin(),
                            t.expectedUcis.end(),
                            u) != t.expectedUcis.end();
        if (!ok) {
            std::cerr << "SearchTest FAILED (" << t.desc
                      << "): got " << u << ", expected "
                      << t.expectedUcis[0] << "\n";
            return 1;
        }
    }
    std::cout << "Simple capture tests passed\n";

    // --- Time-management test: zero ms left should return best-so-far  ---
    b.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    TimeManager tmZero;
    tmZero.start(0, /*inc=*/0, /*moves_to_go=*/1);
    Move bestZero = s.findBestMove(b, b.sideToMove(), 5, tmZero);
    std::string uZero = bestZero.toString();
    if (uZero != "a2a3") {
        std::cerr << "SearchTest FAILED (Time expired): got "
                  << uZero << ", expected a1a1\n";
        return 1;
    }
    std::cout << "Time management test passed\n";

    std::cout << "All SearchTests passed\n";
    return 0;
}
