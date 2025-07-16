#pragma once
#include <chrono>

class TimeManager {
public:
    /**
     * Initialize the timer with clock settings.
     * @param millis_left Total remaining time in milliseconds.
     * @param increment   Per-move increment in milliseconds.
     * @param moves_to_go Estimated moves to go until time control.
     */
    void start(uint64_t millis_left, uint64_t increment, int moves_to_go);

    /**
     * Query whether our allocated time budget has been exhausted.
     */
    bool isTimeUp() const;

private:
    std::chrono::steady_clock::time_point stop_time_;
};