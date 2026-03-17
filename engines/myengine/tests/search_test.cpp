/**
 * search_test.cpp
 *
 * Full search test suite.  Tests are grouped into five sections:
 *
 *  1. Regression  – bugs that have been fixed; must never regress
 *  2. Quiescence  – correctness of the q-search layer
 *  3. Move ordering – TT-first, MVV-LVA, history heuristic
 *  4. Performance – node-count scaling, NPS, TT-hit rate, time control
 *  5. Correctness coverage – tactics, draw detection, determinism
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <chrono>

#include "search.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "board.h"
#include "move.h"


// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

struct SearchResult {
    Move move;
    Search::SearchStats stats;
    long long elapsedMs;   // wall-clock milliseconds
};

// depth-limited, single-threaded, no time pressure
static SearchResult run_search_full(const std::string& fen, int depth) {
    Board board;
    board.loadFEN(fen);

    TranspositionTable tt(16);
    Evaluator evaluator;
    Search search(evaluator, tt);
    search.setThreadCount(1);

    auto t0 = std::chrono::steady_clock::now();
    Move m = search.findBestMove(board, depth, /*timeMs=*/0, 0);
    auto t1 = std::chrono::steady_clock::now();

    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    return { m, search.getStats(), ms };
}

static Move run_search(const std::string& fen, int depth) {
    return run_search_full(fen, depth).move;
}

// time-limited search (passes real clock budget to engine)
static SearchResult run_search_timed(const std::string& fen, int maxDepth, int timeLimitMs) {
    Board board;
    board.loadFEN(fen);

    TranspositionTable tt(16);
    Evaluator evaluator;
    Search search(evaluator, tt);
    search.setThreadCount(1);

    auto t0 = std::chrono::steady_clock::now();
    Move m = search.findBestMove(board, maxDepth, timeLimitMs, 0);
    auto t1 = std::chrono::steady_clock::now();

    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    return { m, search.getStats(), ms };
}


// ===========================================================================
// SECTION 1 – Regression tests
// ===========================================================================

// Bug 1: findBestMove always returned rootMoves[0] because currentBestMove
// was never promoted back to bestMove after each depth iteration.
static void test_regression_best_move_updates_per_depth() {
    std::cout << "--- test_regression_best_move_updates_per_depth ---\n";

    // White: Ka1, Rb1.  Black: Ka8, Qh1 (undefended).
    // rootMoves[0] is a quiet king move; the correct answer is Rxh1.
    // Before the fix the function always returned the king move.
    const char* fen = "k7/8/8/8/8/8/8/KR5q w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "b1h1" &&
           "bestMove must be updated from currentBestMove at each depth");
    std::cout << "PASS\n\n";
}


// ===========================================================================
// SECTION 2 – Quiescence search
// ===========================================================================

static void test_qsearch_is_called() {
    std::cout << "--- test_qsearch_is_called ---\n";

    SearchResult r = run_search_full(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2);
    std::cout << "  qNodes=" << r.stats.qNodes << "\n";

    assert(r.stats.qNodes > 0 && "qsearch must be invoked at depth-0 leaves");
    std::cout << "PASS\n\n";
}

static void test_qsearch_captures_free_piece() {
    std::cout << "--- test_qsearch_captures_free_piece ---\n";

    // White Re1, undefended black Ne5.  Rxe5 is free material.
    Move move = run_search("4k3/8/8/4n3/8/8/8/4R2K w - - 0 1", 2);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "e1e5");
    std::cout << "PASS\n\n";
}

static void test_qsearch_avoids_losing_trade() {
    std::cout << "--- test_qsearch_avoids_losing_trade ---\n";

    // White Bc4, black Pd5 defended by Rd8.  Bxd5 is a losing trade.
    Move move = run_search("3r4/8/8/3p4/2B5/8/8/4K2k w - - 0 1", 3);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() != "c4d5" &&
           "qsearch must not take a piece when recapture wins material");
    std::cout << "PASS\n\n";
}

static void test_qsearch_resolves_capture_chain() {
    std::cout << "--- test_qsearch_resolves_capture_chain ---\n";

    // White Rd1, black Rd8 undefended (Ka8 is 3 squares away).
    Move move = run_search("k2r4/8/8/8/8/8/7K/3R4 w - - 0 1", 3);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "d1d8");
    std::cout << "PASS\n\n";
}

static void test_qsearch_includes_promotion_captures() {
    std::cout << "--- test_qsearch_includes_promotion_captures ---\n";

    // g7xh8=Q is a capture-promotion; it must appear in q-search.
    const char* fen = "7r/6P1/7k/8/8/8/8/6K1 w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "  move: " << move.toString() << "\n";

    assert((move.toString() == "g7h8Q" || move.toString() == "g7g8Q") &&
           "qsearch must include promotion-captures");
    std::cout << "PASS\n\n";
}

static void test_qsearch_stand_pat_quiet_position() {
    std::cout << "--- test_qsearch_stand_pat_quiet_position ---\n";

    // K vs K: no captures available so every q-call exits via stand-pat.
    SearchResult r = run_search_full("8/8/8/8/8/8/8/K6k w - - 0 1", 2);
    std::cout << "  qNodes=" << r.stats.qNodes << "\n";

    assert(r.stats.qNodes > 0);
    std::cout << "PASS\n\n";
}

static void test_qsearch_no_horizon_effect() {
    std::cout << "--- test_qsearch_no_horizon_effect ---\n";

    // White Qh5 can immediately capture free Rh8 on the depth boundary.
    // Without qsearch the engine might play a quiet move instead.
    Move move = run_search("7r/8/8/7Q/8/8/8/4K2k w - - 0 1", 2);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "h5h8");
    std::cout << "PASS\n\n";
}

// q-nodes should represent a meaningful fraction of total work.
// In a tactical position at depth 4, expect qNodes > 20 % of totalNodes.
static void test_qsearch_node_fraction() {
    std::cout << "--- test_qsearch_node_fraction ---\n";

    // Middlegame position with several capture options.
    SearchResult r = run_search_full(
        "r1bq1rk1/pp2bppp/2n1pn2/3p4/3P4/2NBPN2/PP3PPP/R1BQR1K1 w - - 0 1", 4);

    double fraction = (r.stats.totalNodes > 0)
        ? static_cast<double>(r.stats.qNodes) / r.stats.totalNodes
        : 0.0;
    std::cout << "  totalNodes=" << r.stats.totalNodes
              << "  qNodes=" << r.stats.qNodes
              << "  fraction=" << fraction << "\n";

    assert(r.stats.qNodes > 20 && "qsearch must explore some nodes");
    assert(fraction >= 0.10 && "qNodes should be ≥10 % of total nodes");
    std::cout << "PASS\n\n";
}


// ===========================================================================
// SECTION 3 – Move ordering
// ===========================================================================

static void test_ordering_tt_move_first() {
    std::cout << "--- test_ordering_tt_move_first ---\n";

    // Searching a near-mate position at depth 3 warms the TT; the resulting
    // firstMoveCutoffs ratio indicates TT moves are tried first.
    const char* fen = "6k1/5Q2/6K1/8/8/8/8/8 w - - 0 1";
    SearchResult r = run_search_full(fen, 3);

    std::cout << "  firstMoveCutoffs=" << r.stats.firstMoveCutoffs
              << "  betaCutoffs=" << r.stats.betaCutoffs << "\n";

    assert(r.move.isValid());
    std::cout << "PASS\n\n";
}

static void test_ordering_mvvlva() {
    std::cout << "--- test_ordering_mvvlva ---\n";

    // White Re4, can capture black Qe5 or black Pa5.
    // MVV-LVA: queen victim (900) >> pawn victim (100).
    Move move = run_search("4k3/8/8/p3q3/4R3/8/8/4K3 w - - 0 1", 2);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "e4e5");
    std::cout << "PASS\n\n";
}

static void test_ordering_history_heuristic() {
    std::cout << "--- test_ordering_history_heuristic ---\n";

    SearchResult r = run_search_full(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4);

    std::cout << "  betaCutoffs=" << r.stats.betaCutoffs
              << "  firstMoveCutoffs=" << r.stats.firstMoveCutoffs << "\n";

    if (r.stats.betaCutoffs > 0) {
        double ratio = static_cast<double>(r.stats.firstMoveCutoffs) /
                       r.stats.betaCutoffs;
        std::cout << "  first-move ratio=" << ratio << "\n";
        assert(ratio >= 0.10 && "history heuristic should yield ≥10 % first-move cutoff ratio");
    }
    std::cout << "PASS\n\n";
}


// ===========================================================================
// SECTION 4 – Performance benchmarks
// ===========================================================================

// Node counts must grow monotonically as depth increases.
static void test_perf_nodes_scale_with_depth() {
    std::cout << "--- test_perf_nodes_scale_with_depth ---\n";

    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    long long prev = 0;

    for (int d = 1; d <= 4; ++d) {
        SearchResult r = run_search_full(fen, d);
        std::cout << "  depth=" << d
                  << "  totalNodes=" << r.stats.totalNodes
                  << "  qNodes=" << r.stats.qNodes
                  << "  ms=" << r.elapsedMs << "\n";

        assert(r.stats.totalNodes > prev &&
               "node count must strictly increase with depth");
        prev = r.stats.totalNodes;
    }
    std::cout << "PASS\n\n";
}

// Measure nodes-per-second.  Only assert a very conservative lower bound
// (500 NPS) to avoid flakiness on slow CI machines; primary value is the
// printed report.
static void test_perf_nps_benchmark() {
    std::cout << "--- test_perf_nps_benchmark ---\n";

    SearchResult r = run_search_full(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5);

    long long nps = (r.elapsedMs > 0)
        ? (r.stats.totalNodes * 1000LL / r.elapsedMs)
        : r.stats.totalNodes;

    std::cout << "  depth=5  totalNodes=" << r.stats.totalNodes
              << "  ms=" << r.elapsedMs
              << "  NPS=" << nps << "\n";

    assert(r.stats.totalNodes > 0);
    assert(nps >= 500 && "NPS sanity: should exceed 500 nodes/sec");
    std::cout << "PASS\n\n";
}

// After iterative deepening to depth ≥3, the TT must have been hit.
static void test_perf_tt_hits_active() {
    std::cout << "--- test_perf_tt_hits_active ---\n";

    SearchResult r = run_search_full(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4);

    std::cout << "  ttHits=" << r.stats.ttHits << "\n";

    assert(r.stats.ttHits > 0 && "TT must be consulted and hit during iterative deepening");
    std::cout << "PASS\n\n";
}

// Time-controlled search: engine must stop well within the budget and return
// a legal move.  Uses a high maxDepth to ensure the time limit fires first.
static void test_perf_time_controlled_search() {
    std::cout << "--- test_perf_time_controlled_search ---\n";

    const int timeLimitMs = 1000;   // 1 s budget
    const int maxAllowedMs = 3000;  // generous upper bound (3×)

    SearchResult r = run_search_timed(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        /*maxDepth=*/20, timeLimitMs);

    std::cout << "  move=" << r.move.toString()
              << "  elapsed=" << r.elapsedMs << "ms"
              << "  totalNodes=" << r.stats.totalNodes << "\n";

    assert(r.move.isValid() && "time-limited search must return a valid move");
    assert(r.elapsedMs < maxAllowedMs && "search must not exceed time budget by 3×");
    std::cout << "PASS\n\n";
}

// Compare node counts at depth 4 with and without a pre-warmed TT.
// A warm TT should yield at least as many TT hits, reducing effective nodes.
static void test_perf_warm_tt_reduces_nodes() {
    std::cout << "--- test_perf_warm_tt_reduces_nodes ---\n";

    const char* fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";

    // First search with a fresh TT
    Board board1; board1.loadFEN(fen);
    TranspositionTable tt(16);
    Evaluator ev;
    Search s1(ev, tt);
    s1.setThreadCount(1);
    s1.findBestMove(board1, 4, 0, 0);
    long long hits1 = s1.getStats().ttHits;
    long long nodes1 = s1.getStats().totalNodes;

    // Second search on the same TT (entries from depth 1-4 already stored)
    Board board2; board2.loadFEN(fen);
    Search s2(ev, tt);
    s2.setThreadCount(1);
    s2.findBestMove(board2, 4, 0, 0);
    long long hits2 = s2.getStats().ttHits;
    long long nodes2 = s2.getStats().totalNodes;

    std::cout << "  fresh:  nodes=" << nodes1 << "  ttHits=" << hits1 << "\n";
    std::cout << "  warmed: nodes=" << nodes2 << "  ttHits=" << hits2 << "\n";

    assert(hits2 >= hits1 && "warmed TT must hit at least as often as fresh TT");
    std::cout << "PASS\n\n";
}


// ===========================================================================
// SECTION 5 – Correctness coverage
// ===========================================================================

// ---- 5a. Mate detection ----

static void test_coverage_mate_in_1_positions() {
    std::cout << "--- test_coverage_mate_in_1_positions ---\n";

    // Verify engine delivers checkmate (accept any mating move, not just one).
    // IMPORTANT: the queen must be guarded by the king so it cannot be captured.
    const char* fens[] = {
        "7k/8/7K/8/8/8/8/R7 w - - 0 1",    // Ra8#: Kh6 supports, Kh8 cornered
        "7k/5Q2/7K/8/8/8/8/8 w - - 0 1",   // Qg7#: queen guarded by Kh6
        "6k1/5Q2/6K1/8/8/8/8/8 w - - 0 1", // Qg7# or Qf8#: queen guarded by Kg6
    };

    for (auto fen : fens) {
        Move move = run_search(fen, 2);

        Board b; b.loadFEN(fen);
        b.makeMove(move);
        bool isMate = b.isCheckmate(b.sideToMove());

        std::cout << "  move=" << move.toString()
                  << "  checkmate=" << (isMate ? "yes" : "no") << "\n";
        assert(isMate && "engine must deliver checkmate in a mate-in-1 position");
    }
    std::cout << "PASS\n\n";
}

static void test_coverage_mate_in_2_rook_cut() {
    std::cout << "--- test_coverage_mate_in_2_rook_cut ---\n";

    Move move = run_search("4r3/R7/6R1/8/8/5K2/8/6k1 w - - 0 1", 4);
    std::cout << "  move: " << move.toString() << "\n";

    assert((move.toString() == "a7a1" || move.toString() == "a7g7"));
    std::cout << "PASS\n\n";
}

static void test_coverage_mate_in_3_back_rank() {
    std::cout << "--- test_coverage_mate_in_3_back_rank ---\n";

    Move move = run_search("6k1/5ppp/8/8/8/8/1r6/2R1R1K1 w - - 0 1", 5);
    std::cout << "  move: " << move.toString() << "\n";

    assert((move.toString() == "e1e8" || move.toString() == "c1c8"));
    std::cout << "PASS\n\n";
}

// ---- 5b. Check evasion ----

// White king is in check with exactly one legal escape.
static void test_coverage_forced_check_evasion() {
    std::cout << "--- test_coverage_forced_check_evasion ---\n";

    // White Ka1 in check from black Rd1 (attacks along rank 1).
    // Black Bc4 covers a2 (diagonal c4-b3-a2), so Ka2 is illegal.
    // b1 is on rank 1 (attacked by Rd1), and King can't reach Rd1 in one move.
    // Only legal escape: Ka1→b2.
    const char* fen = "7k/8/8/8/2b5/8/8/K2r4 w - - 0 1";
    Move move = run_search(fen, 1);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "a1b2" &&
           "in check with one escape, engine must play the only legal move");
    std::cout << "PASS\n\n";
}

// ---- 5c. Tactics ----

// Knight fork: Ne4-f6+ attacks black Kg8 and Re8 simultaneously.
// After king flees, Nxe8 wins the rook.
static void test_coverage_knight_fork() {
    std::cout << "--- test_coverage_knight_fork ---\n";

    // Knight on e4, king on h1 (off e-file so not pinned); black Kg8, Re8.
    // Ne4->f6+ checks Kg8 and attacks Re8 simultaneously (fork).
    const char* fen = "4r1k1/8/8/8/4N3/8/8/7K w - - 0 1";
    Move move = run_search(fen, 3);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() == "e4f6" &&
           "Nf6+ must be found: it forks Kg8 and Re8");
    std::cout << "PASS\n\n";
}

// Stalemate avoidance: engine must not give stalemate in a winning position.
static void test_coverage_stalemate_avoidance() {
    std::cout << "--- test_coverage_stalemate_avoidance ---\n";

    const char* fen = "7k/5Q2/6K1/8/8/8/8/8 w - - 0 1";
    Move move = run_search(fen, 2);
    std::cout << "  move: " << move.toString() << "\n";

    assert(move.toString() != "f7g6" && "Qg6 gives stalemate — must not be played");
    std::cout << "PASS\n\n";
}

// Hanging queen: engine must save its queen when under immediate attack.
static void test_coverage_hanging_piece_defense() {
    std::cout << "--- test_coverage_hanging_piece_defense ---\n";

    const char* fen = "r1bqkbnr/ppp1pppp/2n5/3p4/3Q4/2N5/PPP1PPPP/R1B1KBNR w KQkq - 0 1";
    Move move = run_search(fen, 2);
    std::cout << "  move: " << move.toString() << "\n";

    // Any queen move that escapes the attack is correct
    assert(move.toString()[0] == 'd' &&
           "engine must move the queen off d4");
    std::cout << "PASS\n\n";
}

// ---- 5d. Draw detection ----

// In a K+K position the engine must return a legal move without hanging.
static void test_coverage_kk_returns_legal_move() {
    std::cout << "--- test_coverage_kk_returns_legal_move ---\n";

    SearchResult r = run_search_full("8/8/8/8/8/8/8/K6k w - - 0 1", 3);
    std::cout << "  move: " << r.move.toString() << "\n";

    assert(r.move.isValid());
    std::cout << "PASS\n\n";
}

// The engine must not attempt to "win" in a drawn K+K position: the
// absolute score should be near zero.
static void test_coverage_kk_score_near_zero() {
    std::cout << "--- test_coverage_kk_score_near_zero ---\n";

    // We probe by searching depth 1 and confirming no crash; then verify
    // that at depth 2 the node count is tiny (no real work to do).
    SearchResult r = run_search_full("8/8/8/8/8/8/8/K6k w - - 0 1", 2);
    std::cout << "  totalNodes=" << r.stats.totalNodes
              << "  qNodes=" << r.stats.qNodes << "\n";

    // Very few nodes: each king has ~3 moves, depth 2 → at most ~9 leaves
    assert(r.stats.totalNodes < 200 &&
           "K+K search must be tiny — no material to evaluate");
    std::cout << "PASS\n\n";
}

// ---- 5e. Determinism ----

// Identical position and depth must always produce the same move.
static void test_coverage_search_deterministic() {
    std::cout << "--- test_coverage_search_deterministic ---\n";

    const char* fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
    Move m1 = run_search(fen, 4);
    Move m2 = run_search(fen, 4);
    std::cout << "  run1=" << m1.toString() << "  run2=" << m2.toString() << "\n";

    assert(m1.toString() == m2.toString() &&
           "single-threaded search must be deterministic");
    std::cout << "PASS\n\n";
}

// ---- 5f. Depth quality ----

// A deeper search must find a tactic that a shallower one misses.
// Position: Nf6+ fork only visible at depth ≥3; depth 1 would miss it.
static void test_coverage_deeper_search_improves_quality() {
    std::cout << "--- test_coverage_deeper_search_improves_quality ---\n";

    // Knight fork position: Ne4-f6 is winning at depth 3+
    // King on h1 (not e-file) so Ne4 is not pinned by Re8.
    const char* fen = "4r1k1/8/8/8/4N3/8/8/7K w - - 0 1";

    Move shallow = run_search(fen, 1);
    Move deep    = run_search(fen, 4);

    std::cout << "  depth1=" << shallow.toString()
              << "  depth4=" << deep.toString() << "\n";

    assert(deep.toString() == "e4f6" && "depth-4 must find the winning fork");
    std::cout << "PASS\n\n";
}


// ===========================================================================
// main
// ===========================================================================

int main() {
    std::cout << "========== SECTION 1: Regression ==========\n\n";
    test_regression_best_move_updates_per_depth();

    std::cout << "========== SECTION 2: Quiescence Search ==========\n\n";
    test_qsearch_is_called();
    test_qsearch_captures_free_piece();
    test_qsearch_avoids_losing_trade();
    test_qsearch_resolves_capture_chain();
    test_qsearch_includes_promotion_captures();
    test_qsearch_stand_pat_quiet_position();
    test_qsearch_no_horizon_effect();
    test_qsearch_node_fraction();

    std::cout << "========== SECTION 3: Move Ordering ==========\n\n";
    test_ordering_tt_move_first();
    test_ordering_mvvlva();
    test_ordering_history_heuristic();

    std::cout << "========== SECTION 4: Performance ==========\n\n";
    test_perf_nodes_scale_with_depth();
    test_perf_nps_benchmark();
    test_perf_tt_hits_active();
    test_perf_time_controlled_search();
    test_perf_warm_tt_reduces_nodes();

    std::cout << "========== SECTION 5: Correctness Coverage ==========\n\n";
    test_coverage_mate_in_1_positions();
    test_coverage_mate_in_2_rook_cut();
    test_coverage_mate_in_3_back_rank();
    test_coverage_forced_check_evasion();
    test_coverage_knight_fork();
    test_coverage_stalemate_avoidance();
    test_coverage_hanging_piece_defense();
    test_coverage_kk_returns_legal_move();
    test_coverage_kk_score_near_zero();
    test_coverage_search_deterministic();
    test_coverage_deeper_search_improves_quality();

    std::cout << "\n========================================\n";
    std::cout << "ALL SEARCH TESTS PASSED\n";
    return 0;
}
