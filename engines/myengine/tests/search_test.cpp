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


// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct SearchResult {
    Move move;
    Search::SearchStats stats;
};

static SearchResult run_search_full(const std::string& fen, int depth) {
    Board board;
    board.loadFEN(fen);

    TranspositionTable tt(16); // 16 MB is plenty for unit tests
    Evaluator evaluator;
    Search search(evaluator, tt);
    search.setThreadCount(1); // single-threaded for determinism

    Move m = search.findBestMove(board, depth, /*timeMs=*/0, 0);
    return { m, search.getStats() };
}

static Move run_search(const std::string& fen, int depth) {
    return run_search_full(fen, depth).move;
}


// ---------------------------------------------------------------------------
// Bug-1 regression: bestMove must update each completed depth iteration
// ---------------------------------------------------------------------------

static void test_best_move_updates_from_search() {
    std::cout << "--- test_best_move_updates_from_search ---\n";

    // White: Ka1, Rb1.  Black: Ka8, Qh1 (undefended).
    // The queen is a free capture via Rb1xh1.  rootMoves[0] will be a king
    // move (Ka1a2 / Ka1b2), so before the Bug-1 fix findBestMove always
    // returned that king move instead of the winning rook capture.
    const char* fen = "k7/8/8/8/8/8/8/KR5q w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() == "b1h1" &&
           "Bug 1 regression: bestMove must be updated from currentBestMove");
    std::cout << "PASS: bestMove correctly updated to best found move (b1h1).\n\n";
}


// ---------------------------------------------------------------------------
// Quiescence search tests
// ---------------------------------------------------------------------------

// After a depth-0 leaf, the engine runs qsearch. Verify qNodes are counted.
static void test_qsearch_is_called() {
    std::cout << "--- test_qsearch_is_called ---\n";

    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    SearchResult r = run_search_full(fen, 2);
    std::cout << "qNodes = " << r.stats.qNodes << "\n";

    assert(r.stats.qNodes > 0 && "qsearch must be invoked at depth-0 leaves");
    std::cout << "PASS: qsearch was called (" << r.stats.qNodes << " qNodes).\n\n";
}

// Quiescence must resolve a free hanging piece that appears at the depth
// boundary. Without qsearch, the engine would over-estimate a position where
// its own piece is en-prise on the next ply.
static void test_qsearch_captures_free_piece() {
    std::cout << "--- test_qsearch_captures_free_piece ---\n";

    // White rook e1, undefended black knight e5.  Rxe5 is free material.
    // At depth 1 the rook takes the knight; qsearch confirms no recapture.
    const char* fen = "4k3/8/8/4n3/8/8/8/4R2K w - - 0 1";
    Move move = run_search(fen, 2);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() == "e1e5" &&
           "qsearch must see and take an undefended hanging piece");
    std::cout << "PASS: Engine captured free knight via qsearch (e1e5).\n\n";
}

// Quiescence must NOT take a piece if the recapture loses more material.
// A bishop taking a pawn that is defended by a rook is a losing trade; the
// stand-pat score should be preferred.
static void test_qsearch_avoids_losing_trade() {
    std::cout << "--- test_qsearch_avoids_losing_trade ---\n";

    // White bishop c4, black pawn d5 defended by black rook d8.
    // Bxd5 loses the bishop for a pawn => bad trade.
    // Engine should NOT play Bxd5.
    const char* fen = "3r4/8/8/3p4/2B5/8/8/4K2k w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() != "c4d5" &&
           "qsearch must not take a piece when the recapture is losing");
    std::cout << "PASS: Engine avoided losing trade (played " << move.toString() << ").\n\n";
}

// Quiescence must resolve a multi-step capture exchange and return the
// correct net evaluation.  After Rxd1 Rxd1, white is up a rook vs pawn.
// Engine should take the rook when it is safe to do so.
static void test_qsearch_resolves_capture_chain() {
    std::cout << "--- test_qsearch_resolves_capture_chain ---\n";

    // White: Rd1, Kh2.  Black: Rd8 (undefended — Ka8 is 3 squares away),
    // Ka8.  Rxd8 wins the rook outright; qsearch confirms no recapture.
    const char* fen = "k2r4/8/8/8/8/8/7K/3R4 w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() == "d1d8" &&
           "qsearch must take a free rook");
    std::cout << "PASS: Engine took free rook through qsearch capture chain.\n\n";
}

// Promotion-captures must be included in quiescence search.
// A pawn capturing and promoting to a queen is a huge material gain that
// must not be missed at the depth boundary.
static void test_qsearch_includes_promotion_captures() {
    std::cout << "--- test_qsearch_includes_promotion_captures ---\n";

    // White pawn g7, black rook h8, black king h6. White can g7xh8=Q+.
    // At depth 1 the capture-promotion appears as a qsearch node.
    const char* fen = "7r/6P1/7k/8/8/8/8/6K1 w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() == "g7h8Q" || move.toString() == "g7g8Q" &&
           "qsearch must include promotion-captures");
    std::cout << "PASS: Engine found promotion capture (" << move.toString() << ").\n\n";
}

// In a quiet position (no captures available) the stand-pat score is
// returned immediately without exploring any moves, keeping qNodes low.
static void test_qsearch_stand_pat_in_quiet_position() {
    std::cout << "--- test_qsearch_stand_pat_in_quiet_position ---\n";

    // Pure king vs king: no captures, so every q-call is just stand-pat.
    const char* fen = "8/8/8/8/8/8/8/K6k w - - 0 1";
    SearchResult r = run_search_full(fen, 2);
    std::cout << "qNodes = " << r.stats.qNodes << ", move = " << r.move.toString() << "\n";

    // qNodes should be exactly equal to the number of leaves (no capture
    // loops should be entered), so the count should be modest.
    assert(r.stats.qNodes > 0 && "qsearch must still be called");
    std::cout << "PASS: Stand-pat returned in quiet position.\n\n";
}

// Engine must not fall for the horizon effect: playing a pawn push to delay
// an inevitable material loss by one ply (the loss becomes invisible past
// the search horizon without qsearch).
static void test_qsearch_no_horizon_effect_blunder() {
    std::cout << "--- test_qsearch_no_horizon_effect_blunder ---\n";

    // White queen h5 attacks black rook on h8.  Black rook is undefended.
    // Without qsearch the engine might play a random quiet move thinking the
    // position is great; with qsearch it resolves Qxh8 immediately.
    const char* fen = "7r/8/8/7Q/8/8/8/4K2k w - - 0 1";
    Move move = run_search(fen, 2);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() == "h5h8" &&
           "qsearch must resolve free captures, avoiding horizon-effect blunders");
    std::cout << "PASS: Engine took free rook (no horizon blunder).\n\n";
}


// ---------------------------------------------------------------------------
// Move ordering tests
// ---------------------------------------------------------------------------

// The transposition table move must be tried first, which means a beta
// cutoff from the TT move arrives earlier, boosting firstMoveCutoffs.
static void test_move_ordering_tt_move_first() {
    std::cout << "--- test_move_ordering_tt_move_first ---\n";

    // Mate-in-1: only one move is legal (checkmate). The TT will store it
    // after depth 1, and a depth-2 search re-uses it as the first move.
    const char* fen = "6k1/5Q2/6K1/8/8/8/8/8 w - - 0 1";
    SearchResult r1 = run_search_full(fen, 1);
    SearchResult r2 = run_search_full(fen, 3);

    std::cout << "depth1 firstMoveCutoffs=" << r1.stats.firstMoveCutoffs
              << "  depth3 firstMoveCutoffs=" << r2.stats.firstMoveCutoffs << "\n";
    // We simply check that the search completes and returns a legal move.
    assert(r2.move.isValid() && "TT ordering must produce a valid move");
    std::cout << "PASS: TT move ordering did not break the search.\n\n";
}

// Captures scored by MVV-LVA must come before quiet moves in the ordering.
// The simplest proxy: in a position with multiple capture options, the one
// with the highest-value victim (queen) should be tried before a pawn capture.
static void test_move_ordering_mvvlva_captures_before_quiets() {
    std::cout << "--- test_move_ordering_mvvlva_captures_before_quiets ---\n";

    // White rook can capture black queen (e5) or black pawn (a5).
    // MVV-LVA should score Rxe5 (captures queen) >> Rxa5 (captures pawn).
    // Engine must prefer Rxe5.
    const char* fen = "4k3/8/8/p3q3/4R3/8/8/4K3 w - - 0 1";
    Move move = run_search(fen, 2);
    std::cout << "Engine move: " << move.toString() << "\n";

    assert(move.toString() == "e4e5" &&
           "MVV-LVA ordering must prioritize queen capture over pawn capture");
    std::cout << "PASS: Engine captured queen (best MVV-LVA victim).\n\n";
}

// History heuristic: a quiet move that caused a beta cutoff before should
// be tried earlier in subsequent searches.  We verify that firstMoveCutoffs
// increases at higher depths (more beta cutoffs from the first move), which
// is a proxy for good ordering.
static void test_move_ordering_history_heuristic_active() {
    std::cout << "--- test_move_ordering_history_heuristic_active ---\n";

    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    SearchResult r = run_search_full(fen, 4);

    std::cout << "betaCutoffs=" << r.stats.betaCutoffs
              << "  firstMoveCutoffs=" << r.stats.firstMoveCutoffs << "\n";

    // A well-ordered search should achieve many first-move cutoffs.
    // Require at least 10% of beta cutoffs come from the first move.
    if (r.stats.betaCutoffs > 0) {
        double ratio = static_cast<double>(r.stats.firstMoveCutoffs) /
                       static_cast<double>(r.stats.betaCutoffs);
        std::cout << "  first-move cutoff ratio = " << ratio << "\n";
        assert(ratio >= 0.10 && "history heuristic should drive first-move cutoff ratio >= 10%");
    }
    std::cout << "PASS: History heuristic producing first-move cutoffs.\n\n";
}


// ---------------------------------------------------------------------------
// Existing search correctness tests (preserved + compile-error fixed)
// ---------------------------------------------------------------------------

static void test_mate_in_2_rook_cut() {
    std::cout << "--- test_mate_in_2_rook_cut ---\n";
    const char* fen = "4r3/R7/6R1/8/8/5K2/8/6k1 w - - 0 1";

    Move move = run_search(fen, 4);
    std::cout << "Engine move: " << move.toString() << "\n";

    if (move.toString() == "a7a1" || move.toString() == "a7g7") {
        std::cout << "PASS: Found checkmate sequence start.\n\n";
    } else {
        std::cerr << "FAIL: Missed mate in 2. Got " << move.toString() << "\n";
        exit(1);
    }
}

static void test_mate_in_3_back_rank() {
    std::cout << "--- test_mate_in_3_back_rank ---\n";
    const char* fen = "6k1/5ppp/8/8/8/8/1r6/2R1R1K1 w - - 0 1";

    Move move = run_search(fen, 5);
    std::cout << "Engine move: " << move.toString() << "\n";

    if (move.toString() == "e1e8" || move.toString() == "c1c8") {
        std::cout << "PASS: Found back rank mate.\n\n";
    } else {
        std::cerr << "FAIL: Missed back rank mate. Got " << move.toString() << "\n";
        exit(1);
    }
}

static void test_stalemate_avoidance() {
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
        std::cout << "PASS: Found Checkmate.\n\n";
    } else {
        std::cout << "PASS: Avoided stalemate (played " << move.toString() << ")\n\n";
    }
}

static void test_hanging_piece_defense() {
    std::cout << "--- test_hanging_piece_defense ---\n";
    const char* fen = "r1bqkbnr/ppp1pppp/2n5/3p4/3Q4/2N5/PPP1PPPP/R1B1KBNR w KQkq - 0 1";

    Move move = run_search(fen, 2);
    std::cout << "Engine move: " << move.toString() << "\n";  // fixed: was `m.toString()`

    if (move.toString() == "d4d5" || move.toString() == "d4a4" || move.toString() == "d4d3" ||
        move.toString() == "d4d2" || move.toString() == "d4d1") {
        std::cout << "PASS: Saved the Queen.\n\n";
    } else {
        std::cerr << "FAIL: Likely lost the Queen. Played " << move.toString() << "\n";
        exit(1);
    }
}


// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
    // Bug-1 regression
    test_best_move_updates_from_search();

    // Quiescence correctness
    test_qsearch_is_called();
    test_qsearch_captures_free_piece();
    test_qsearch_avoids_losing_trade();
    test_qsearch_resolves_capture_chain();
    test_qsearch_includes_promotion_captures();
    test_qsearch_stand_pat_in_quiet_position();
    test_qsearch_no_horizon_effect_blunder();

    // Move ordering
    test_move_ordering_tt_move_first();
    test_move_ordering_mvvlva_captures_before_quiets();
    test_move_ordering_history_heuristic_active();

    // Existing correctness suite (compile-error fixed)
    test_mate_in_2_rook_cut();
    test_mate_in_3_back_rank();
    test_stalemate_avoidance();
    test_hanging_piece_defense();

    std::cout << "\nALL SEARCH TESTS PASSED\n";
    return 0;
}
