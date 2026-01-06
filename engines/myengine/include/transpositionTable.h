#pragma once

#include <cstdint>
#include <unordered_map>
#include "move.h"

class TranspositionTable {
public:
    static constexpr int EXACT = 0;
    static constexpr int LOWERBOUND = 1;
    static constexpr int UPPERBOUND = 2;

    struct TTEntry {
        int value;
        int depth;
        Move bestMove;
        int flag;
        static constexpr int EXACT = TranspositionTable::EXACT;
        static constexpr int LOWERBOUND = TranspositionTable::LOWERBOUND;
        static constexpr int UPPERBOUND = TranspositionTable::UPPERBOUND;
    };

    TranspositionTable(size_t buckets = 0) {
        if (buckets) table_.reserve(buckets);
    }

    void store(uint64_t key, int value, int depth, Move bestMove, int flag);
    bool probe(uint64_t key, TTEntry& out) const;

    struct ProbeResult {
        bool found;
        int value;
        int depth;
        Move bestMove;
        int flag;
    };

    ProbeResult probe(uint64_t key) const;

private:
    std::unordered_map<uint64_t, TTEntry> table_;
};
