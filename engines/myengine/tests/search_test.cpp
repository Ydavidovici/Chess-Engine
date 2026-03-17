#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

#include "search.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "board.h"
#include "move.h"


Move run_search(const std::string& fen, int depth) {
    Board board;
    board.loadFEN(fen);

    TranspositionTable tt(10000);
    Evaluator evaluator;
    Search search(evaluator, tt);

    return search.findBestMove(board, depth, 5000, 0);
}

void test_mate_in_2_rook_cut() {
    std::cout << "--- test_mate_in_2_rook_cut ---\n";
    const char* fen = "4r3/R7/6R1/8/8/5K2/8/6k1 w - - 0 1";

    Move move = run_search(fen, 4);
    std::cout << "Engine move: " << move.toString() << "\n";

    if (move.toString() == "a7a1" || move.toString() == "a7g7") {
        std::cout << "PASS: Found checkmate sequence start.\n";
    } else {
        std::cerr << "FAIL: Missed mate in 2. Got " << move.toString() << "\n";
        exit(1);
    }
}

void test_mate_in_3_back_rank() {
    std::cout << "--- test_mate_in_3_back_rank ---\n";
    const char* fen = "6k1/5ppp/8/8/8/8/1r6/2R1R1K1 w - - 0 1";

    Move move = run_search(fen, 5);
    std::cout << "Engine move: " << move.toString() << "\n";

    if (move.toString() == "e1e8" || move.toString() == "c1c8") {
        std::cout << "PASS: Found back rank mate.\n";
    } else {
        std::cerr << "FAIL: Missed back rank mate. Got " << move.toString() << "\n";
        exit(1);
    }
}

void test_stalemate_avoidance() {
    std::cout << "--- test_stalemate_avoidance ---\n";

    const char* fen = "7k/5Q2/6K1/8/8/8/8/8 w - - 0 1";
    Move move = run_search(fen, 2);

    std::cout << "Position: " << fen << "\n";
    std::cout << "Engine move: " << move.toString() << "\n";

    if (move.toString() == "f7g6") {
        std::cerr << "FAIL: Engine played Stalemate (Qg6) in a winning position!\n";
        exit(1);
    }
    if (move.toString() == "f7g7" || move.toString() == "f7f8" || move.toString() == "f7h7") {
        std::cout << "PASS: Found Checkmate.\n";
    } else {
        std::cout << "PASS: Avoided stalemate (played " << move.toString() << ")\n";
    }
}

void test_hanging_piece_defense() {
    std::cout << "--- test_hanging_piece_defense ---\n";
    const char* fen = "r1bqkbnr/ppp1pppp/2n5/3p4/3Q4/2N5/PPP1PPPP/R1B1KBNR w KQkq - 0 1";

    Move move = run_search(fen, 2);
    std::cout << "Engine move: " << m.toString() << "\n";


    if (move.toString() == "d4d5" || move.toString() == "d4a4" || move.toString() == "d4d3" ||
        move.toString() == "d4d2" || move.toString() == "d4d1") {
        std::cout << "PASS: Saved the Queen.\n";
    } else {
        std::cerr << "FAIL: Likely lost the Queen. Played " << move.toString() << "\n";
        exit(1);
    }
}

int main() {
    test_mate_in_2_rook_cut();
    test_mate_in_3_back_rank();
    test_stalemate_avoidance();
    test_hanging_piece_defense();

    std::cout << "\nALL SEARCH TESTS PASSED\n";
    return 0;
}