#include "timeManager.h"
#include <algorithm>

void TimeManager::start(uint64_t millis_left, uint64_t inc, int mtg) {
    auto safety = std::chrono::milliseconds(50);
    auto alloc = std::chrono::milliseconds(
        millis_left > safety.count()
          ? (millis_left - safety.count()) / std::max(1, mtg)
          : 0
    ) + std::chrono::milliseconds(inc);
    stop_time_ = std::chrono::steady_clock::now() + alloc;
}

bool TimeManager::isTimeUp() const {
    return std::chrono::steady_clock::now() >= stop_time_;
}