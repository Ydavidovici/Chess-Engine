// engine/test_engine.cpp
#include "engine.h"
#include "move.h"
#include "types.h"
#include <iostream>
#include <string>

int main() {
    Engine e;

    std::cout << "Initial position:\n";
    e.printBoard();

    // Test loading a FEN
    std::string fen = "r2Bk2r/p4pb1/n3p1p1/1p5p/3pP3/PbPP1N1P/1P1N1PP1/R3KBR1 w Q - 0 21";
    std::cout << "\nLoading FEN: " << fen << "\n";
    e.loadFEN(fen);

    std::cout << "\nAfter loadFEN:\n";
    e.printBoard();

    // Static evaluation
    int eval = e.evaluateBoard();
    std::cout << "\nStatic eval: " << eval << "\n";

    // Find best move from this position (depth 3, White to move)
    std::cout << "\nSearching best move (depth 3, WHITE):\n";
    Move best = e.findBestMove(3, Color::WHITE);
    std::cout << "Best move = " << best.toString() << "\n";

    return 0;
}