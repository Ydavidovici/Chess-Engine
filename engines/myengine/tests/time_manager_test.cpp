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

// =========== SECTION 1: Immediate timeout ===========

static void test_zero_time_zero_inc_is_immediately_up() {
    std::cout << "--- test_zero_time_zero_inc_is_immediately_up ---\n";
    TimeManager tm;
    tm.start(0, 0, 0);
    REQUIRE_MSG(tm.isSoftTimeUp(), "alloc=0ms: soft must be up immediately");
    REQUIRE_MSG(tm.isHardTimeUp(), "alloc=0ms: hard must be up immediately");
    std::cout << "PASS\n\n";
}

static void test_sub_safety_time_is_immediately_up() {
    std::cout << "--- test_sub_safety_time_is_immediately_up ---\n";
    TimeManager tm;
    tm.start(25, 0, 0);
    REQUIRE_MSG(tm.isSoftTimeUp(), "25ms < safety: soft up immediately");
    REQUIRE_MSG(tm.isHardTimeUp(), "25ms < safety: hard up immediately");
    std::cout << "PASS\n\n";
}

static void test_exactly_safety_boundary_is_immediately_up() {
    std::cout << "--- test_exactly_safety_boundary_is_immediately_up ---\n";
    TimeManager tm;
    tm.start(50, 0, 0);
    REQUIRE_MSG(tm.isSoftTimeUp(), "millis_left==safety: soft up immediately");
    REQUIRE_MSG(tm.isHardTimeUp(), "millis_left==safety: hard up immediately");
    std::cout << "PASS\n\n";
}

static void test_zero_time_with_increment_not_up() {
    std::cout << "--- test_zero_time_with_increment_not_up ---\n";
    TimeManager tm;
    tm.start(0, 500, 0);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "inc=500ms: soft must not be up immediately");
    REQUIRE_MSG(!tm.isHardTimeUp(), "inc=500ms: hard must not be up immediately");
    std::cout << "PASS\n\n";
}

// =========== SECTION 2: Not yet expired ===========

static void test_large_budget_not_up_immediately() {
    std::cout << "--- test_large_budget_not_up_immediately ---\n";
    TimeManager tm;
    tm.start(60000, 0, 0);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "60s budget must not be up immediately");
    REQUIRE_MSG(!tm.isHardTimeUp(), "60s budget must not be up immediately");
    std::cout << "PASS\n\n";
}

static void test_large_budget_not_up_after_short_sleep() {
    std::cout << "--- test_large_budget_not_up_after_short_sleep ---\n";
    TimeManager tm;
    tm.start(10000, 0, 0);  // mtg=0 -> DEFAULT_MTG=45 -> soft ~220ms, hard ~1100ms
    sleep_ms(100);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "10s/30 budget: soft not up at 100ms");
    REQUIRE_MSG(!tm.isHardTimeUp(), "10s/30 budget: hard not up at 100ms");
    std::cout << "PASS\n\n";
}

static void test_increment_raises_allocation() {
    std::cout << "--- test_increment_raises_allocation ---\n";
    TimeManager tm;
    tm.start(5050, 1000, 0);  // DEFAULT_MTG=45 -> soft ~110 + 0.8*1000 = ~910ms
    sleep_ms(100);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "soft~1167ms: not up at 100ms");
    std::cout << "PASS\n\n";
}

// =========== SECTION 3: Allocation formula (explicit mtg) ===========

static void test_explicit_mtg_divides_time() {
    std::cout << "--- test_explicit_mtg_divides_time ---\n";
    TimeManager tm;
    tm.start(1050, 0, 10);  // soft = 1000/10 = 100ms
    sleep_ms(30);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "soft~100ms: not up at 30ms");
    sleep_ms(300);
    REQUIRE_MSG(tm.isSoftTimeUp(), "soft~100ms: up by 330ms");
    std::cout << "PASS\n\n";
}

static void test_mtg_large_yields_small_slice() {
    std::cout << "--- test_mtg_large_yields_small_slice ---\n";
    TimeManager tm;
    tm.start(10050, 0, 100);  // soft = 10000/100 = 100ms
    sleep_ms(30);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "100ms slice: not up at 30ms");
    sleep_ms(300);
    REQUIRE_MSG(tm.isSoftTimeUp(), "100ms slice: up by 330ms");
    std::cout << "PASS\n\n";
}

static void test_explicit_mtg_increment_added() {
    std::cout << "--- test_explicit_mtg_increment_added ---\n";
    TimeManager tm;
    tm.start(1050, 200, 10);  // soft = 95 + 0.8*200 = 255ms
    sleep_ms(100);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "soft~300ms: not up at 100ms");
    sleep_ms(500);
    REQUIRE_MSG(tm.isSoftTimeUp(), "soft~300ms: up by 600ms");
    std::cout << "PASS\n\n";
}

static void test_sub_safety_remaining_uses_increment_only() {
    std::cout << "--- test_sub_safety_remaining_uses_increment_only ---\n";
    TimeManager tm;
    tm.start(30, 300, 0);  // adj clamps to 0; soft = 0 + 0.8*300 = 240ms
    sleep_ms(100);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "inc-only 300ms: not up at 100ms");
    sleep_ms(500);
    REQUIRE_MSG(tm.isSoftTimeUp(), "inc-only 300ms: up by 600ms");
    std::cout << "PASS\n\n";
}

// =========== SECTION 4: Default mtg substitution ===========

static void test_no_mtg_uses_default_not_full_remaining() {
    std::cout << "--- test_no_mtg_uses_default_not_full_remaining ---\n";
    // 30s clock with mtg=0 should NOT burn the whole 30s.
    // With DEFAULT_MTG=45: soft = (30000-100)/45 ~= 664ms.
    TimeManager tm;
    tm.start(30000, 0, 0);
    sleep_ms(200);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "default mtg=30: soft~1s, not up at 200ms");
    sleep_ms(1500);  // 1700ms total — well past 1s soft target
    REQUIRE_MSG(tm.isSoftTimeUp(),
                "default mtg=30: soft~1s, MUST be up by 1700ms "
                "(regression for sudden-death burn-clock bug)");
    std::cout << "PASS\n\n";
}

static void test_no_mtg_default_with_increment() {
    std::cout << "--- test_no_mtg_default_with_increment ---\n";
    // 30s + 500ms inc, mtg=0 -> soft ~= 664 + 0.8*500 = ~1064ms
    TimeManager tm;
    tm.start(30000, 500, 0);
    sleep_ms(200);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "soft~1.5s: not up at 200ms");
    sleep_ms(2000);  // 2200ms
    REQUIRE_MSG(tm.isSoftTimeUp(), "soft~1.5s: up by 2200ms");
    std::cout << "PASS\n\n";
}

static void test_mtg_zero_uses_default_mtg() {
    std::cout << "--- test_mtg_zero_uses_default_mtg ---\n";
    // mtg=0 must behave like an explicit mtg=DEFAULT_MTG (=45).
    TimeManager tm0, tm30;
    tm0.start(30000, 0, 0);
    tm30.start(30000, 0, TimeManager::DEFAULT_MTG);
    sleep_ms(500);
    REQUIRE_MSG(tm0.isSoftTimeUp() == tm30.isSoftTimeUp(),
                "mtg=0 and mtg=DEFAULT_MTG must yield identical soft deadlines");
    std::cout << "PASS\n\n";
}

static void test_mtg_negative_uses_default_mtg() {
    std::cout << "--- test_mtg_negative_uses_default_mtg ---\n";
    TimeManager tmN, tm30;
    tmN.start(30000, 0, -5);
    tm30.start(30000, 0, TimeManager::DEFAULT_MTG);
    sleep_ms(500);
    REQUIRE_MSG(tmN.isSoftTimeUp() == tm30.isSoftTimeUp(),
                "mtg<0 must be treated as DEFAULT_MTG");
    std::cout << "PASS\n\n";
}

static void test_explicit_mtg_overrides_default() {
    std::cout << "--- test_explicit_mtg_overrides_default ---\n";
    // mtg=60 -> soft ~= 30000/60 = 500ms, much shorter than default.
    TimeManager tm;
    tm.start(30000, 0, 60);
    sleep_ms(300);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "soft~500ms: not up at 300ms");
    sleep_ms(700);  // 1000ms total
    REQUIRE_MSG(tm.isSoftTimeUp(), "soft~500ms: up by 1000ms");
    std::cout << "PASS\n\n";
}

// =========== SECTION 5: Hard deadline ===========

static void test_hard_deadline_exceeds_soft_by_default() {
    std::cout << "--- test_hard_deadline_exceeds_soft_by_default ---\n";
    // No stability info -> hard is MAX_FACTOR(2) * soft, capped at max_spend.
    // start(30000, 0, 60): soft~498ms, hard=min(996, 0.8*29900)=996ms.
    TimeManager tm;
    tm.start(30000, 0, 60);
    sleep_ms(700);  // past soft (500), before hard (2500)
    REQUIRE_MSG(tm.isSoftTimeUp(), "soft must fire first");
    REQUIRE_MSG(!tm.isHardTimeUp(), "hard must NOT fire at 700ms (hard~2500ms)");
    std::cout << "PASS\n\n";
}

static void test_hard_deadline_capped_by_remaining() {
    std::cout << "--- test_hard_deadline_capped_by_remaining ---\n";
    // mtg=1 with 1050ms left: base=950, max_spend=0.8*950=760ms.
    // Both soft and hard cap at 760ms -> hard cannot exceed remaining.
    TimeManager tm;
    tm.start(1050, 0, 1);
    sleep_ms(900);
    REQUIRE_MSG(tm.isHardTimeUp(), "hard must be capped at <=800ms, up by 900ms");
    std::cout << "PASS\n\n";
}

// =========== SECTION 6: Stability scaling ===========

static void test_stability_shrinks_soft_deadline() {
    std::cout << "--- test_stability_shrinks_soft_deadline ---\n";
    // mtg=10, 2050ms -> soft ~195ms. Shrunk (0.5x) ~97ms.
    TimeManager tm;
    tm.start(2050, 0, 10);
    tm.onIterationComplete(false);
    tm.onIterationComplete(false);
    tm.onIterationComplete(false);
    sleep_ms(140);  // > 100ms shrunk, < 200ms normal
    REQUIRE_MSG(tm.isSoftTimeUp(),
                "after 3 stable iterations, shrunk soft (~100ms) must be up at 140ms");
    REQUIRE_MSG(!tm.isHardTimeUp(),
                "hard (~1000ms) must still be active");
    std::cout << "PASS\n\n";
}

static void test_change_extends_soft_deadline() {
    std::cout << "--- test_change_extends_soft_deadline ---\n";
    // mtg=10, 2050ms -> soft ~195ms, extended (1.5x) ~292ms.
    TimeManager tm;
    tm.start(2050, 0, 10);
    tm.onIterationComplete(true);   // changed -> scale = 1.5
    sleep_ms(220);
    REQUIRE_MSG(!tm.isSoftTimeUp(),
                "after change, extended soft (~300ms) must NOT fire at 220ms");
    sleep_ms(200);  // 420ms total
    REQUIRE_MSG(tm.isSoftTimeUp(),
                "extended soft (~300ms) must fire by 420ms");
    std::cout << "PASS\n\n";
}

static void test_change_resets_stable_counter() {
    std::cout << "--- test_change_resets_stable_counter ---\n";
    TimeManager tm;
    tm.start(2050, 0, 10);  // soft ~195ms
    tm.onIterationComplete(false);
    tm.onIterationComplete(false);
    tm.onIterationComplete(false);  // shrunk to ~97ms
    tm.onIterationComplete(true);   // extended to ~292ms; counter reset
    sleep_ms(220);
    REQUIRE_MSG(!tm.isSoftTimeUp(),
                "extension after stability must NOT fire at 220ms");
    std::cout << "PASS\n\n";
}

static void test_stability_never_exceeds_hard() {
    std::cout << "--- test_stability_never_exceeds_hard ---\n";
    // mtg=1 with 1050ms: soft caps at max_spend=760ms, hard also =760ms.
    // EXTEND_SCALE * soft would be 1140ms, but isSoftTimeUp clamps to hard.
    TimeManager tm;
    tm.start(1050, 0, 1);
    tm.onIterationComplete(true);
    sleep_ms(850);
    REQUIRE_MSG(tm.isSoftTimeUp(),
                "scaled soft must never exceed hard deadline");
    std::cout << "PASS\n\n";
}

static void test_one_stable_iteration_does_not_shrink() {
    std::cout << "--- test_one_stable_iteration_does_not_shrink ---\n";
    TimeManager tm;
    tm.start(2050, 0, 10);  // soft = 200ms
    tm.onIterationComplete(false);  // count=1, scale stays 1.0
    sleep_ms(140);
    REQUIRE_MSG(!tm.isSoftTimeUp(),
                "only 1 stable iteration: scale stays 1.0, soft~200ms");
    std::cout << "PASS\n\n";
}

// =========== SECTION 7: Reset on re-start() ===========

static void test_reset_to_short_budget() {
    std::cout << "--- test_reset_to_short_budget ---\n";
    TimeManager tm;
    tm.start(60000, 0, 0);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "large budget: not up");
    tm.start(0, 0, 0);
    REQUIRE_MSG(tm.isSoftTimeUp(), "after reset to 0ms: up immediately");
    std::cout << "PASS\n\n";
}

static void test_reset_to_long_budget() {
    std::cout << "--- test_reset_to_long_budget ---\n";
    TimeManager tm;
    tm.start(0, 0, 0);
    REQUIRE_MSG(tm.isSoftTimeUp(), "0ms: up immediately");
    tm.start(60000, 0, 0);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "after reset to 60s: not up");
    std::cout << "PASS\n\n";
}

static void test_restart_clears_stability_state() {
    std::cout << "--- test_restart_clears_stability_state ---\n";
    TimeManager tm;
    tm.start(2050, 0, 10);
    tm.onIterationComplete(false);
    tm.onIterationComplete(false);
    tm.onIterationComplete(false);  // shrunk
    tm.start(2050, 0, 10);  // restart -> stable_count=0, scale=1.0
    sleep_ms(140);
    REQUIRE_MSG(!tm.isSoftTimeUp(),
                "restart must reset stable_count and soft_scale");
    std::cout << "PASS\n\n";
}

// =========== SECTION 8: Search integration ===========

static void test_search_returns_within_time_budget() {
    std::cout << "--- test_search_returns_within_time_budget ---\n";
    SearchResult r = run_timed(MIDDLEGAME, 500);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 2500.0, "500ms budget: finish within 2500ms (5x)");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_returns_within_tight_budget() {
    std::cout << "--- test_search_returns_within_tight_budget ---\n";
    SearchResult r = run_timed(STARTPOS, 100);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 500.0, "100ms budget: finish within 500ms (5x)");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_returns_valid_move_when_budget_near_zero() {
    std::cout << "--- test_search_returns_valid_move_when_budget_near_zero ---\n";
    SearchResult r = run_timed(STARTPOS, 50);
    REQUIRE_MSG(r.move.isValid(),
                "near-zero budget: depth-1 fallback must produce a valid move");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_uses_increment() {
    std::cout << "--- test_search_uses_increment ---\n";
    SearchResult r = run_timed(STARTPOS, 10, 500, 0);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 2500.0, "inc=500ms: finish within 2500ms");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

static void test_search_movestogo_restricts_time() {
    std::cout << "--- test_search_movestogo_restricts_time ---\n";
    SearchResult r = run_timed(STARTPOS, 10000, 0, 20);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 2500.0,
                "mtg=20: soft ~497ms, elapsed must be < 2500ms");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

// Regression: UCI 'go movetime N' must allocate ~N ms, not divide N by
// DEFAULT_MTG. Previously movetime=5000 routed through start(5000,0,0)
// and yielded soft~165ms / hard~825ms — causing the Lichess bot to use
// almost none of its allocated time per move.
static void test_startFixed_uses_full_budget() {
    std::cout << "--- test_startFixed_uses_full_budget ---\n";
    TimeManager tm;
    tm.startFixed(500);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "movetime=500ms: soft must not be up immediately");
    REQUIRE_MSG(!tm.isHardTimeUp(), "movetime=500ms: hard must not be up immediately");
    sleep_ms(200);
    REQUIRE_MSG(!tm.isSoftTimeUp(),
                "movetime=500ms: soft must not fire at 200ms (regression: previously fired at ~16ms)");
    sleep_ms(400);  // 600ms total
    REQUIRE_MSG(tm.isSoftTimeUp(), "movetime=500ms: soft must fire by 600ms");
    REQUIRE_MSG(tm.isHardTimeUp(), "movetime=500ms: hard must fire by 600ms");
    std::cout << "PASS\n\n";
}

static void test_startFixed_tiny_budget_immediate() {
    std::cout << "--- test_startFixed_tiny_budget_immediate ---\n";
    TimeManager tm;
    tm.startFixed(30);  // below SAFETY_MS=50
    REQUIRE_MSG(tm.isSoftTimeUp(), "movetime<safety: soft up immediately");
    REQUIRE_MSG(tm.isHardTimeUp(), "movetime<safety: hard up immediately");
    std::cout << "PASS\n\n";
}

// Regression: 'go movetime N' through findBestMove must spend close to N ms,
// not N/30. Previously a 500ms movetime would return in ~50-100ms.
static void test_search_movetime_uses_full_budget() {
    std::cout << "--- test_search_movetime_uses_full_budget ---\n";
    Board board;
    board.loadFEN(MIDDLEGAME);
    TranspositionTable tt(16);
    Evaluator ev;
    Search search(ev, tt);
    search.setThreadCount(1);

    auto t0 = std::chrono::steady_clock::now();
    Move m = search.findBestMove(board, /*maxDepth=*/20, /*timeLeftMs=*/0, /*incMs=*/0, /*mtg=*/0, /*movetimeMs=*/500);
    double ms = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - t0).count() / 1000.0;

    REQUIRE_MSG(m.isValid(), "must return a valid move");
    REQUIRE_MSG(ms >= 300.0,
                "movetime=500ms: must spend at least 300ms thinking "
                "(regression for movetime-divided-by-mtg bug)");
    REQUIRE_MSG(ms < 1500.0, "movetime=500ms: must not exceed 1500ms");
    std::cout << "  elapsed=" << ms << "ms  move=" << m.toString() << "\n";
    std::cout << "PASS\n\n";
}

// Regression: with mtg=0 (sudden death) and 30s remaining, the engine
// must NOT spend the whole 30s on a single move.
static void test_search_sudden_death_does_not_burn_clock() {
    std::cout << "--- test_search_sudden_death_does_not_burn_clock ---\n";
    SearchResult r = run_timed(STARTPOS, 30000, 0, 0);
    REQUIRE_MSG(r.move.isValid(), "must return a valid move");
    REQUIRE_MSG(r.elapsedMs < 5000.0,
                "30s sudden-death budget: with DEFAULT_MTG=45 the engine "
                "should spend well under 1s, not the full 30s. Elapsed < 5s.");
    std::cout << "  elapsed=" << r.elapsedMs << "ms  move=" << r.move.toString() << "\n";
    std::cout << "PASS\n\n";
}

// =========== SECTION 10: Self-scaling allocation across time controls ===========
// One formula must yield sane, monotonically larger budgets from bullet to
// classical and never commit more than MAX_FRACTION of the clock (plus the
// increment) to a single move. Checked deterministically via softMs/hardMs.

static int64_t maxSpend(int64_t clock, int64_t inc,
                        int overhead = TimeManager::DEFAULT_OVERHEAD_MS) {
    const int64_t adj = clock > overhead ? clock - overhead : 0;
    return static_cast<int64_t>(adj * TimeManager::MAX_FRACTION) + inc;
}

static void test_alloc_scales_across_time_controls() {
    std::cout << "--- test_alloc_scales_across_time_controls ---\n";
    TimeManager bullet, blitz, rapid, classical;
    bullet.start(60000, 0, 0);           // 1+0
    blitz.start(180000, 2000, 0);        // 3+2
    rapid.start(600000, 0, 0);           // 10+0
    classical.start(1800000, 20000, 0);  // 30+20

    REQUIRE_MSG(bullet.softMs() < blitz.softMs(), "bullet soft < blitz soft");
    REQUIRE_MSG(blitz.softMs() < rapid.softMs(), "blitz soft < rapid soft");
    REQUIRE_MSG(rapid.softMs() < classical.softMs(), "rapid soft < classical soft");

    REQUIRE_MSG(bullet.hardMs() >= bullet.softMs(), "bullet hard >= soft");
    REQUIRE_MSG(blitz.hardMs() >= blitz.softMs(), "blitz hard >= soft");
    REQUIRE_MSG(rapid.hardMs() >= rapid.softMs(), "rapid hard >= soft");
    REQUIRE_MSG(classical.hardMs() >= classical.softMs(), "classical hard >= soft");

    REQUIRE_MSG(bullet.hardMs() <= maxSpend(60000, 0), "bullet within ceiling");
    REQUIRE_MSG(blitz.hardMs() <= maxSpend(180000, 2000), "blitz within ceiling");
    REQUIRE_MSG(rapid.hardMs() <= maxSpend(600000, 0), "rapid within ceiling");
    REQUIRE_MSG(classical.hardMs() <= maxSpend(1800000, 20000), "classical within ceiling");

    std::cout << "  bullet soft=" << bullet.softMs() << " hard=" << bullet.hardMs() << "\n";
    std::cout << "  blitz  soft=" << blitz.softMs() << " hard=" << blitz.hardMs() << "\n";
    std::cout << "  rapid  soft=" << rapid.softMs() << " hard=" << rapid.hardMs() << "\n";
    std::cout << "  class  soft=" << classical.softMs() << " hard=" << classical.hardMs() << "\n";
    std::cout << "PASS\n\n";
}

static void test_single_move_never_exceeds_fraction_of_clock() {
    std::cout << "--- test_single_move_never_exceeds_fraction_of_clock ---\n";
    // Severe scramble: tiny clock, no increment. Even the hard deadline must
    // stay under MAX_FRACTION of the clock so we cannot flag.
    TimeManager tm;
    tm.start(1000, 0, 0);
    REQUIRE_MSG(tm.hardMs() <= maxSpend(1000, 0), "hard <= 0.8*clock in scramble");
    REQUIRE_MSG(tm.softMs() <= tm.hardMs(), "soft <= hard");
    std::cout << "  soft=" << tm.softMs() << " hard=" << tm.hardMs() << "\n";
    std::cout << "PASS\n\n";
}

// =========== SECTION 11: Dynamic scaling — score-drop panic & overhead ===========

static void test_score_drop_triggers_panic_extension() {
    std::cout << "--- test_score_drop_triggers_panic_extension ---\n";
    TimeManager tm;
    tm.start(60000, 0, 0);
    tm.onIterationComplete(false, 40);   // baseline score
    tm.onIterationComplete(false, 35);   // tiny change: no panic, stable
    REQUIRE_MSG(tm.currentScale() != TimeManager::PANIC_SCALE,
                "small score change must not trigger panic");
    tm.onIterationComplete(false, -30);  // 65cp drop >= PANIC_DROP_CP
    REQUIRE_MSG(tm.currentScale() == TimeManager::PANIC_SCALE,
                "sharp score drop must extend (panic) even on a stable move");
    std::cout << "PASS\n\n";
}

static void test_panic_clamped_by_hard_deadline() {
    std::cout << "--- test_panic_clamped_by_hard_deadline ---\n";
    // Panic raises the soft scale, but isSoftTimeUp must still clamp to hard.
    TimeManager tm;
    tm.start(1050, 0, 1);  // soft caps at max_spend; hard == max_spend
    tm.onIterationComplete(false, 100);
    tm.onIterationComplete(true, -100);  // changed + huge drop -> panic
    sleep_ms(static_cast<int>(tm.hardMs()) + 60);
    REQUIRE_MSG(tm.isSoftTimeUp(), "panic-scaled soft must never exceed hard");
    std::cout << "PASS\n\n";
}

static void test_startFixed_ignores_stability_scaling() {
    std::cout << "--- test_startFixed_ignores_stability_scaling ---\n";
    // Fixed movetime is deterministic: onIterationComplete must be a no-op.
    TimeManager tm;
    tm.startFixed(1000);
    const double before = tm.currentScale();
    tm.onIterationComplete(true, -10000);  // would normally panic-extend
    tm.onIterationComplete(false, 0);
    tm.onIterationComplete(false, 0);
    tm.onIterationComplete(false, 0);      // would normally shrink
    REQUIRE_MSG(tm.currentScale() == before,
                "startFixed must ignore PV-stability scaling");
    std::cout << "PASS\n\n";
}

static void test_predictive_soft_stops_early_for_long_next_iter() {
    std::cout << "--- test_predictive_soft_stops_early_for_long_next_iter ---\n";
    TimeManager tm;
    tm.start(2050, 0, 10);  // soft ~195ms, hard ~390ms
    sleep_ms(100);
    REQUIRE_MSG(!tm.isSoftTimeUp(), "plain soft not up at 100ms (soft~195)");
    REQUIRE_MSG(tm.isSoftTimeUp(200),
                "predicted 200ms next iter would overshoot soft -> stop now");
    REQUIRE_MSG(!tm.isSoftTimeUp(10),
                "predicted 10ms next iter still fits before soft");
    std::cout << "PASS\n\n";
}

static void test_overhead_reduces_budget() {
    std::cout << "--- test_overhead_reduces_budget ---\n";
    TimeManager small, large;
    small.setOverhead(0);
    large.setOverhead(1000);
    small.start(11000, 0, 10);  // base = 11000/10 = 1100
    large.start(11000, 0, 10);  // base = (11000-1000)/10 = 1000
    REQUIRE_MSG(large.softMs() < small.softMs(),
                "larger overhead must shrink the budget");
    REQUIRE_MSG(small.softMs() - large.softMs() >= 90,
                "1000ms overhead over mtg=10 should cost ~100ms of budget");
    std::cout << "  overhead0 soft=" << small.softMs()
              << "  overhead1000 soft=" << large.softMs() << "\n";
    std::cout << "PASS\n\n";
}

int main() {
    std::cout << "========== SECTION 1: Immediate Timeout ==========\n\n";
    test_zero_time_zero_inc_is_immediately_up();
    test_sub_safety_time_is_immediately_up();
    test_exactly_safety_boundary_is_immediately_up();
    test_zero_time_with_increment_not_up();

    std::cout << "========== SECTION 2: Not Yet Expired ==========\n\n";
    test_large_budget_not_up_immediately();
    test_large_budget_not_up_after_short_sleep();
    test_increment_raises_allocation();

    std::cout << "========== SECTION 3: Allocation Formula (explicit mtg) ==========\n\n";
    test_explicit_mtg_divides_time();
    test_mtg_large_yields_small_slice();
    test_explicit_mtg_increment_added();
    test_sub_safety_remaining_uses_increment_only();

    std::cout << "========== SECTION 4: Default mtg Substitution ==========\n\n";
    test_no_mtg_uses_default_not_full_remaining();
    test_no_mtg_default_with_increment();
    test_mtg_zero_uses_default_mtg();
    test_mtg_negative_uses_default_mtg();
    test_explicit_mtg_overrides_default();

    std::cout << "========== SECTION 5: Hard Deadline ==========\n\n";
    test_hard_deadline_exceeds_soft_by_default();
    test_hard_deadline_capped_by_remaining();

    std::cout << "========== SECTION 6: Stability Scaling ==========\n\n";
    test_stability_shrinks_soft_deadline();
    test_change_extends_soft_deadline();
    test_change_resets_stable_counter();
    test_stability_never_exceeds_hard();
    test_one_stable_iteration_does_not_shrink();

    std::cout << "========== SECTION 7: Reset Behavior ==========\n\n";
    test_reset_to_short_budget();
    test_reset_to_long_budget();
    test_restart_clears_stability_state();

    std::cout << "========== SECTION 8: Search Integration ==========\n\n";
    test_search_returns_within_time_budget();
    test_search_returns_within_tight_budget();
    test_search_returns_valid_move_when_budget_near_zero();
    test_search_uses_increment();
    test_search_movestogo_restricts_time();
    test_search_sudden_death_does_not_burn_clock();

    std::cout << "========== SECTION 9: Fixed Movetime ==========\n\n";
    test_startFixed_uses_full_budget();
    test_startFixed_tiny_budget_immediate();
    test_search_movetime_uses_full_budget();

    std::cout << "========== SECTION 10: Self-Scaling Allocation ==========\n\n";
    test_alloc_scales_across_time_controls();
    test_single_move_never_exceeds_fraction_of_clock();

    std::cout << "========== SECTION 11: Score-Drop Panic & Overhead ==========\n\n";
    test_score_drop_triggers_panic_extension();
    test_panic_clamped_by_hard_deadline();
    test_startFixed_ignores_stability_scaling();
    test_predictive_soft_stops_early_for_long_next_iter();
    test_overhead_reduces_budget();

    std::cout << "\n========================================\n";
    std::cout << "ALL TIME MANAGER TESTS PASSED\n";
    return 0;
}
