#include <iostream>
#include <cmath>
#include <cassert>
#include "evaluator.h"
#include "board.h"

void assert_score(const Evaluator& ev, Board& b, Color stm, int min_val, int max_val, const char* label) {
    int score = ev.evaluate(b, stm);
    if (score < min_val || score > max_val) {
        std::cerr << "FAIL " << label << ": Score " << score
            << " not in range [" << min_val << ", " << max_val << "]\n";
        exit(1);
    }
    std::cout << "PASS " << label << " (Score: " << score << ")\n";
}

void test_material() {
    std::cout << "--- test_material ---\n";
    Evaluator ev;
    Board b;

    b.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert_score(ev, b, Color::WHITE, -20, 20, "Start Position (Equal)");

    b.loadFEN("rnbqkbnr/1ppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert_score(ev, b, Color::WHITE, 80, 150, "White up 1 Pawn");
    assert_score(ev, b, Color::BLACK, -150, -80, "Black down 1 Pawn (Negamax)");
}

void test_positional() {
    std::cout << "--- test_positional ---\n";
    Evaluator ev;
    Board b;

    b.loadFEN("7k/8/8/8/8/8/8/7N w - - 0 1");
    int scoreBad = ev.evaluate(b, Color::WHITE);

    b.loadFEN("7k/8/8/8/4N3/8/8/8 w - - 0 1");
    int scoreGood = ev.evaluate(b, Color::WHITE);

    if (scoreGood > scoreBad) {
        std::cout << "PASS PST Logic (Center " << scoreGood << " > Rim " << scoreBad << ")\n";
    }
    else {
        std::cerr << "FAIL PST Logic: Center Knight (" << scoreGood
            << ") not better than Rim (" << scoreBad << ")\n";
        exit(1);
    }
}

void test_symmetry() {
    std::cout << "--- test_symmetry ---\n";
    Evaluator ev;
    Board b;

    b.loadFEN("7k/8/8/8/4P3/8/8/K7 w - - 0 1");
    int whiteScore = ev.evaluate(b, Color::WHITE);

    b.loadFEN("k7/8/8/4p3/8/8/8/7K b - - 0 1");
    int blackScore = ev.evaluate(b, Color::BLACK);

    if (whiteScore == blackScore) {
        std::cout << "PASS Symmetry (White " << whiteScore << " == Black " << blackScore << ")\n";
    }
    else {
        std::cerr << "FAIL Symmetry: Mirrored positions have different scores!\n";
        exit(1);
    }
}

void test_terminal_logic() {
    std::cout << "--- test_terminal_logic ---\n";
    Evaluator ev;
    Board b;

    b.loadFEN("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");

    b.loadFEN("6k1/6Q1/6K1/8/8/8/8/8 b - - 0 1");

    int mateScore = ev.evaluateTerminal(b, Color::BLACK);
    if (mateScore == -100000) std::cout << "PASS Checkmate detection\n";
    else std::cerr << "FAIL Checkmate detection (Got " << mateScore << ")\n";
}

int main() {
    test_material();
    test_positional();
    test_symmetry();
    test_terminal_logic();

    std::cout << "ALL EVAL TESTS PASSED\n";
    return 0;
}