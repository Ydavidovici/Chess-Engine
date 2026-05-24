#include "timeManager.h"
#include <algorithm>

void TimeManager::setOverhead(int ms) {
    overhead_ms_ = (ms < 0) ? 0 : ms;
}

void TimeManager::start(uint64_t millis_left, uint64_t inc, int mtg) {
    const int mtg_eff = (mtg <= 0) ? DEFAULT_MTG : mtg;

    const int64_t adj =
        (millis_left > static_cast<uint64_t>(overhead_ms_))
            ? static_cast<int64_t>(millis_left) - overhead_ms_
            : 0;

    // Absolute ceiling for a single move: never commit more than MAX_FRACTION
    // of the usable clock (plus the increment we immediately earn back). This
    // is the flag guard, and it is what makes one formula safe from bullet to
    // classical.
    const int64_t max_spend =
        static_cast<int64_t>(adj * MAX_FRACTION) + static_cast<int64_t>(inc);

    // Base budget: an even share of the clock over the moves we expect to
    // play, plus most of the increment.
    const int64_t base =
        adj / mtg_eff + static_cast<int64_t>(inc * INC_FRACTION);

    const int64_t soft = std::min(base, max_spend);
    const int64_t hard =
        std::min<int64_t>(static_cast<int64_t>(base * MAX_FACTOR), max_spend);

    start_time_ = std::chrono::steady_clock::now();
    soft_alloc_ = std::chrono::milliseconds(soft);
    hard_alloc_ = std::chrono::milliseconds(hard);
    soft_scale_ = 1.0;
    stable_count_ = 0;
    scaling_enabled_ = true;
    have_prev_score_ = false;
    prev_score_ = 0;
}

void TimeManager::startFixed(uint64_t movetime_ms) {
    const int64_t budget =
        (movetime_ms > static_cast<uint64_t>(SAFETY_MS))
            ? static_cast<int64_t>(movetime_ms) - SAFETY_MS
            : 0;

    start_time_ = std::chrono::steady_clock::now();
    soft_alloc_ = std::chrono::milliseconds(budget);
    hard_alloc_ = std::chrono::milliseconds(budget);
    soft_scale_ = 1.0;
    stable_count_ = 0;
    scaling_enabled_ = false;
    have_prev_score_ = false;
    prev_score_ = 0;
}

std::chrono::milliseconds TimeManager::effectiveSoft() const {
    const int64_t soft = scaling_enabled_
        ? static_cast<int64_t>(soft_alloc_.count() * soft_scale_)
        : soft_alloc_.count();
    return std::min(std::chrono::milliseconds(soft), hard_alloc_);
}

bool TimeManager::isSoftTimeUp() const {
    return std::chrono::steady_clock::now() >= start_time_ + effectiveSoft();
}

bool TimeManager::isSoftTimeUp(int64_t predicted_next_ms) const {
    // Fixed-budget ('go movetime') searches should consume the full budget, so
    // ignore the look-ahead there; it only guards dynamic clock allocation.
    if (!scaling_enabled_ || predicted_next_ms < 0) predicted_next_ms = 0;
    return std::chrono::steady_clock::now() + std::chrono::milliseconds(predicted_next_ms)
           >= start_time_ + effectiveSoft();
}

bool TimeManager::isHardTimeUp() const {
    return std::chrono::steady_clock::now() >= start_time_ + hard_alloc_;
}

void TimeManager::onIterationComplete(bool best_move_changed, int score) {
    if (!scaling_enabled_) return;

    const bool panic =
        have_prev_score_ && (prev_score_ - score) >= PANIC_DROP_CP;
    prev_score_ = score;
    have_prev_score_ = true;

    if (best_move_changed) {
        stable_count_ = 0;
        soft_scale_ = panic ? PANIC_SCALE : EXTEND_SCALE;
    } else {
        ++stable_count_;
        if (panic) {
            soft_scale_ = PANIC_SCALE;
        } else if (stable_count_ >= STABLE_THRESHOLD) {
            soft_scale_ = SHRINK_SCALE;
        } else {
            soft_scale_ = 1.0;
        }
    }
}
