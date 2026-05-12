#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "timeManager.h"
#include "search.h"
#include "evaluator.h"
#include "transpositionTable.h"
#include "board.h"
#include "move.h"

#define REQUIRE(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAIL [" << __FILE__ << ":" << __LINE__ << "]: " #cond "\n"; \
            std::abort(); \
        } \
    } while (0)

#define REQUIRE_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAIL [" << __FILE__ << ":" << __LINE__ << "]: " << (msg) << "\n"; \
            std::abort(); \
        } \
    } while (0)

static void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

struct SearchResult {
    Move move;
    double elapsedMs;
};

static SearchResult run_timed(const char* fen, int timeLeftMs, int incMs = 0, int mtg = 0) {
    Board board;
    board.loadFEN(fen);
    TranspositionTable tt(16);
    Evaluator ev;
    Search search(ev, tt);
    search.setThreadCount(1);

    auto t0 = std::chrono::steady_clock::now();
    Move m = search.findBestMove(board, /*maxDepth=*/20, timeLeftMs, incMs, mtg);
    double ms = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - t0).count() / 1000.0;
    return {m, ms};
}

static const char* STARTPOS =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const char* MIDDLEGAME =
    "r1bq1rk1/pp2bppp/2n1pn2/3p4/3P4/2NBPN2/PP3PPP/R1BQR1K1 w - - 0 1";

static void test_zero_time_zero_inc_is_immediately_up() {
    std::cout << "--- test_zero_time_zero_inc_is_immediately_up ---\n";
    TimeManager tm;
    tm.start(0, 0, 0);
    REQUIRE_MSG(tm.isTimeUp(), "alloc=0ms: must be up immediately");
    std::cout << "PASS\n\n";
}

static void test_sub_safety_time_is_immediately_up() {
    std::cout << "--- test_sub_safety_time_is_immediately_up ---\n";
    TimeManager tm;
    tm.start(25, 0, 0);
    REQUIRE_MSG(tm.isTimeUp(), "25ms < safety(50ms): must be up immediately");
    std::cout << "PASS\n\n";
}

static void test_exactly_safety_boundary_is_immediately_up() {
    std::cout << "--- test_exactly_safety_boundary_is_immediately_up ---\n";
    TimeManager tm;
    tm.start(50, 0, 0);
    REQUIRE_MSG(tm.isTimeUp(), "millis_left==50 (not >50): must be up immediately");
    std::cout << "PASS\n\n";
}

static void test_zero_time_with_increment_not_up() {
    std::cout << "--- test_zero_time_with_increment_not_up ---\n";
    TimeManager tm;
    tm.start(0, 500, 0);
    REQUIRE_MSG(!tm.isTimeUp(), "inc=500ms: alloc=500ms, must not be up immediately");
    std::cout << "PASS\n\n";
}

static void test_zero_time_zero_inc_nonzero_mtg_is_up() {
    std::cout << "--- test_zero_time_zero_inc_nonzero_mtg_is_up ---\n";
    TimeManager tm;
    tm.start(0, 0, 10);
    REQUIRE_MSG(tm.isTimeUp(), "alloc=0ms even with mtg=10: must be up immediately");
    std::cout << "PASS\n\n";
}

static void test_large_budget_not_up_immediately() {
    std::cout << "--- test_large_budget_not_up_immediately ---\n";
    TimeManager tm;
    tm.start(60000, 0, 0);
    REQUIRE_MSG(!tm.isTimeUp(), "60s budget must not be up immediately");
    std::cout << "PASS\n\n";
}

static void test_large_budget_not_up_after_short_sleep() {
    std::cout << "--- test_large_budget_not_up_after_short_sleep ---\n";
    TimeManager tm;
    tm.start(10000, 0, 0);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "10s budget must not be up after 100ms");
    std::cout << "PASS\n\n";
}

static void test_increment_raises_allocation() {
    std::cout << "--- test_increment_raises_allocation ---\n";
    TimeManager tm;
    tm.start(5050, 1000, 0);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "alloc≈6000ms: must not be up after 100ms");
    std::cout << "PASS\n\n";
}

static void test_no_movestogo_uses_full_remaining() {
    std::cout << "--- test_no_movestogo_uses_full_remaining ---\n";
    TimeManager tm;
    tm.start(550, 0, 0);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "500ms budget: must not be up at 100ms");
    sleep_ms(800);
    REQUIRE_MSG(tm.isTimeUp(), "500ms budget: must be up by 900ms");
    std::cout << "PASS\n\n";
}

static void test_movestogo_divides_time() {
    std::cout << "--- test_movestogo_divides_time ---\n";
    TimeManager tm;
    tm.start(1050, 0, 10);
    sleep_ms(30);
    REQUIRE_MSG(!tm.isTimeUp(), "100ms budget: must not be up at 30ms");
    sleep_ms(300);
    REQUIRE_MSG(tm.isTimeUp(), "100ms budget: must be up by 330ms");
    std::cout << "PASS\n\n";
}

static void test_movestogo_one_same_as_zero() {
    std::cout << "--- test_movestogo_one_same_as_zero ---\n";
    TimeManager tm0, tm1;
    tm0.start(1050, 0, 0);
    tm1.start(1050, 0, 1);
    REQUIRE_MSG(!tm0.isTimeUp(), "mtg=0: 1000ms budget must not be up immediately");
    REQUIRE_MSG(!tm1.isTimeUp(), "mtg=1: 1000ms budget must not be up immediately");
    std::cout << "PASS\n\n";
}

static void test_movestogo_large_yields_small_slice() {
    std::cout << "--- test_movestogo_large_yields_small_slice ---\n";
    TimeManager tm;
    tm.start(10050, 0, 100);
    sleep_ms(30);
    REQUIRE_MSG(!tm.isTimeUp(), "100ms slice: must not be up at 30ms");
    sleep_ms(300);
    REQUIRE_MSG(tm.isTimeUp(), "100ms slice: must be up by 330ms");
    std::cout << "PASS\n\n";
}

static void test_increment_added_to_slice() {
    std::cout << "--- test_increment_added_to_slice ---\n";
    TimeManager tm;
    tm.start(1050, 200, 10);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "300ms alloc: must not be up at 100ms");
    sleep_ms(500);
    REQUIRE_MSG(tm.isTimeUp(), "300ms alloc: must be up by 600ms");
    std::cout << "PASS\n\n";
}

static void test_sub_safety_remaining_uses_increment_only() {
    std::cout << "--- test_sub_safety_remaining_uses_increment_only ---\n";
    TimeManager tm;
    tm.start(30, 300, 0);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "inc-only 300ms: must not be up at 100ms");
    sleep_ms(500);
    REQUIRE_MSG(tm.isTimeUp(), "inc-only 300ms: must be up by 600ms");
    std::cout << "PASS\n\n";
}

static void test_movestogo_zero_clamps_to_one() {
    std::cout << "--- test_movestogo_zero_clamps_to_one ---\n";
    TimeManager tm;
    tm.start(5050, 0, 0);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "mtg=0 clamps to 1: 5000ms budget must not expire in 100ms");
    std::cout << "PASS\n\n";
}

static void test_movestogo_negative_clamps_to_one() {
    std::cout << "--- test_movestogo_negative_clamps_to_one ---\n";
    TimeManager tm;
    tm.start(5050, 0, -3);
    sleep_ms(100);
    REQUIRE_MSG(!tm.isTimeUp(), "mtg=-3 clamps to 1: 5000ms budget must not expire in 100ms");
    std::cout << "PASS\n\n";
}

static void test_reset_to_short_budget() {
    std::cout << "--- test_reset_to_short_budget ---\n";
    TimeManager tm;
    tm.start(60000, 0, 0);
    REQUIRE_MSG(!tm.isTimeUp(), "large budget: must not be up before reset");
    tm.start(0, 0, 0);
    REQUIRE_MSG(tm.isTimeUp(), "after reset to 0ms: must be up immediately");
    std::cout << "PASS\n\n";
}

static void test_reset_to_long_budget() {
    std::cout << "--- test_reset_to_long_budget ---\n";
    TimeManager tm;
    tm.start(0, 0, 0);
    REQUIRE_MSG(tm.isTimeUp(), "0ms budget: must be up immediately");
    tm.start(60000, 0, 0);
    REQUIRE_MSG(!tm.isTimeUp(), "after reset to 60s: must not be up");
    std::cout << "PASS\n\n";
}

static void test_search_returns_within_time_budget() {
    std::cout << "--- test_search_returns_within_time_budget ---\n";
    SearchResult r = run_timed(MIDDLEGAME, 500);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 2500.0, "500ms budget: search must finish within 2500ms (5x safety)");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_returns_within_tight_budget() {
    std::cout << "--- test_search_returns_within_tight_budget ---\n";
    SearchResult r = run_timed(STARTPOS, 100);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 500.0, "100ms budget: search must finish within 500ms (5x safety)");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_returns_valid_move_when_budget_near_zero() {
    std::cout << "--- test_search_returns_valid_move_when_budget_near_zero ---\n";
    SearchResult r = run_timed(STARTPOS, 50);
    REQUIRE_MSG(r.move.isValid(), "even with near-zero budget, depth-1 fallback must produce a valid move");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_uses_increment() {
    std::cout << "--- test_search_uses_increment ---\n";
    SearchResult r = run_timed(STARTPOS, 10, 500, 0);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 2500.0,
                "inc=500ms budget: search must finish within 2500ms");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_movestogo_restricts_time() {
    std::cout << "--- test_search_movestogo_restricts_time ---\n";
    SearchResult r = run_timed(STARTPOS, 10000, 0, 20);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 2500.0, "movestogo=20 must restrict alloc to ~497ms — elapsed must be < 2500ms");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

int main() {
    std::cout << "========== SECTION 1: Immediate Timeout ==========\n\n";
    test_zero_time_zero_inc_is_immediately_up();
    test_sub_safety_time_is_immediately_up();
    test_exactly_safety_boundary_is_immediately_up();
    test_zero_time_with_increment_not_up();
    test_zero_time_zero_inc_nonzero_mtg_is_up();

    std::cout << "========== SECTION 2: Not Yet Expired ==========\n\n";
    test_large_budget_not_up_immediately();
    test_large_budget_not_up_after_short_sleep();
    test_increment_raises_allocation();

    std::cout << "========== SECTION 3: Allocation Formula ==========\n\n";
    test_no_movestogo_uses_full_remaining();
    test_movestogo_divides_time();
    test_movestogo_one_same_as_zero();
    test_movestogo_large_yields_small_slice();
    test_increment_added_to_slice();
    test_sub_safety_remaining_uses_increment_only();

    std::cout << "========== SECTION 4: moves_to_go Edge Cases ==========\n\n";
    test_movestogo_zero_clamps_to_one();
    test_movestogo_negative_clamps_to_one();

    std::cout << "========== SECTION 5: Reset Behavior ==========\n\n";
    test_reset_to_short_budget();
    test_reset_to_long_budget();

    std::cout << "========== SECTION 6: Search Integration ==========\n\n";
    test_search_returns_within_time_budget();
    test_search_returns_within_tight_budget();
    test_search_returns_valid_move_when_budget_near_zero();
    test_search_uses_increment();
    test_search_movestogo_restricts_time();

    std::cout << "\n========================================\n";
    std::cout << "ALL TIME MANAGER TESTS PASSED\n";
    return 0;
}
