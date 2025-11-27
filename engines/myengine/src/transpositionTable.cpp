#include "transpositionTable.h"

// store stays the same
void TranspositionTable::store(uint64_t key,
                               int value,
                               int depth,
                               Move bestMove,
                               int flag) {
    TTEntry e{ value, depth, bestMove, flag };
    table_[key] = e;
}

bool TranspositionTable::probe(uint64_t key, TTEntry &out) const {
    auto it = table_.find(key);
    if (it == table_.end()) return false;
    out = it->second;
    return true;
}

TranspositionTable::ProbeResult
TranspositionTable::probe(uint64_t key) const {
    auto it = table_.find(key);
    if (it == table_.end()) {
        return { false, 0, 0, Move(), TranspositionTable::EXACT };
    }
    const TTEntry &e = it->second;
    return { true, e.value, e.depth, e.bestMove, e.flag };
}
