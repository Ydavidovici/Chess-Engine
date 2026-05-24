#pragma once
#include <chrono>
#include <cstdint>

class TimeManager {
public:
    // --- Static allocation tunables ---
    // One self-scaling model covers bullet→classical: the budget is derived
    // from (clock, increment, moves-to-go), so faster controls get smaller
    // slices and slower ones get larger slices with no per-TC special cases.
    static constexpr int    DEFAULT_MTG  = 45;   // assumed moves left when moves-to-go is unset
    static constexpr double INC_FRACTION = 0.8;  // fraction of the increment folded into the budget
    static constexpr double MAX_FACTOR   = 2.0;  // hard deadline = MAX_FACTOR * base (then capped)
    static constexpr double MAX_FRACTION = 0.8;  // a single move never exceeds this slice of the clock
    static constexpr int    DEFAULT_OVERHEAD_MS = 100; // lag/I-O margin subtracted from the clock

    // --- Dynamic (in-search) scaling tunables ---
    static constexpr double EXTEND_SCALE = 1.5;  // best move changed at the last completed depth
    static constexpr double SHRINK_SCALE = 0.5;  // best move stable for STABLE_THRESHOLD depths
    static constexpr double PANIC_SCALE  = 2.0;  // score dropped sharply vs the previous depth
    static constexpr int    PANIC_DROP_CP = 50;  // centipawn drop that triggers a panic extension
    static constexpr int    STABLE_THRESHOLD = 3;

    static constexpr int    SAFETY_MS = 50;      // margin for fixed-budget ('go movetime') searches

    /** Override the per-move overhead (lag) margin. Clamped to >= 0. */
    void setOverhead(int ms);

    /**
     * Initialize the timer with clock settings.
     * @param millis_left Total remaining time in milliseconds.
     * @param increment   Per-move increment in milliseconds.
     * @param moves_to_go Moves until next time control. <= 0 means unset:
     *                   the manager substitutes DEFAULT_MTG.
     */
    void start(uint64_t millis_left, uint64_t increment, int moves_to_go);

    /**
     * Initialize for a fixed-budget search (UCI 'go movetime N').
     * Soft and hard deadlines both equal movetime minus a small safety
     * margin. Stability scaling is disabled so the budget is deterministic.
     */
    void startFixed(uint64_t movetime_ms);

    /**
     * Soft deadline: "don't begin another iteration past this point."
     * For clock-based searches it scales with PV stability and score
     * swings — see onIterationComplete(). Always clamped by the hard deadline.
     */
    bool isSoftTimeUp() const;

    /**
     * Predictive soft check: true if the soft deadline has passed OR a next
     * iteration estimated to take predicted_next_ms would run past it. Lets the
     * search skip a depth it cannot finish in budget — the main defense against
     * a single deep iteration blowing far past the target.
     */
    bool isSoftTimeUp(int64_t predicted_next_ms) const;

    /**
     * Hard deadline: "abort the current iteration immediately."
     * Capped at MAX_FRACTION of the remaining clock so we never flag.
     */
    bool isHardTimeUp() const;

    /**
     * Search calls this after each completed iterative-deepening depth, with
     * the new best move's stability and its score (centipawns, side-to-move).
     * A stable best move (>= STABLE_THRESHOLD repeats) shrinks the soft
     * deadline; a changed best move or a sharp score drop extends it. Every
     * adjustment stays capped by the hard deadline. No-op for startFixed().
     */
    void onIterationComplete(bool best_move_changed, int score = 0);

    // Inspection (UCI info / tests): base soft & hard budgets in ms, and the
    // current PV-stability multiplier applied to the soft deadline.
    int64_t softMs() const { return soft_alloc_.count(); }
    int64_t hardMs() const { return hard_alloc_.count(); }
    double  currentScale() const { return soft_scale_; }

private:
    std::chrono::milliseconds effectiveSoft() const;

    std::chrono::steady_clock::time_point start_time_{};
    std::chrono::milliseconds soft_alloc_{0};
    std::chrono::milliseconds hard_alloc_{0};
    double soft_scale_{1.0};
    int stable_count_{0};
    int overhead_ms_{DEFAULT_OVERHEAD_MS};
    bool scaling_enabled_{true};
    bool have_prev_score_{false};
    int prev_score_{0};
};
