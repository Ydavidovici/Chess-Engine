#include <iostream>
#include <cmath>
#include <cassert>
#include "evaluator.h"
#include "board.h"

void assert_score(const Evaluator& evaluator, Board& board, Color sideToMove, int min_val, int max_val, const char* label) {
    int score = evaluator.evaluate(board, sideToMove);
    if (score < min_val || score > max_val) {
        std::cerr << "FAIL " << label << ": Score " << score
            << " not in range [" << min_val << ", " << max_val << "]\n";
        exit(1);
    }
    std::cout << "PASS " << label << " (Score: " << score << ")\n";
}

void test_material() {
    std::cout << "--- test_material ---\n";
    Evaluator evaluator;
    Board board;

    board.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert_score(evaluator, board, Color::WHITE, -20, 20, "Start Position (Equal)");

    board.loadFEN("rnbqkbnr/1ppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert_score(evaluator, board, Color::WHITE, 80, 150, "White up 1 Pawn");
    assert_score(evaluator, board, Color::BLACK, -150, -80, "Black down 1 Pawn (Negamax)");
}

void test_positional() {
    std::cout << "--- test_positional ---\n";
    Evaluator evaluator;
    Board board;

    board.loadFEN("2qk4/8/8/8/8/8/8/2QK3N w - - 0 1");
    int scoreBad = evaluator.evaluate(board, Color::WHITE);

    board.loadFEN("2qk4/8/8/8/4N3/8/8/2QK4 w - - 0 1");
    int scoreGood = evaluator.evaluate(board, Color::WHITE);

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
    Evaluator evaluator;
    Board board;

    board.loadFEN("3k4/8/8/8/8/8/8/3KQ3 b - - 0 1");
    int whiteScore = evaluator.evaluate(board, Color::WHITE);

    board.loadFEN("3kq3/8/8/8/8/8/8/3K4 b - - 0 1");
    int blackScore = evaluator.evaluate(board, Color::BLACK);

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
    Evaluator evaluator;
    Board board;

    board.loadFEN("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");

    board.loadFEN("6k1/6Q1/6K1/8/8/8/8/8 b - - 0 1");

    int mateScore = evaluator.evaluateTerminal(board, Color::BLACK);
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