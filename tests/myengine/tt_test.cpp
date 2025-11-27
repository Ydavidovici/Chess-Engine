#include <iostream>
#include <cstdlib>
#include "transpositionTable.h"
#include "move.h"

int main(){
    TranspositionTable tt(1024);
    uint64_t key = 0x12345678ULL;
    Move m = Move::fromUCI("e2e4");

    // Store & probe
    tt.store(key, /*value=*/42, /*depth=*/3, m, TranspositionTable::TTEntry::EXACT);

    TranspositionTable::TTEntry out;
    if (!tt.probe(key, out)) {
        std::cerr << "TTTest FAILED: expected probe to find key\n";
        return 1;
    }
    if (out.value != 42 || out.depth != 3 || !(out.bestMove == m) ||
        out.flag != TranspositionTable::TTEntry::EXACT)
    {
        std::cerr << "TTTest FAILED: entry fields mismatch\n";
        return 1;
    }

    // Missing key
    if (tt.probe(key + 1, out)) {
        std::cerr << "TTTest FAILED: probe found non-existent key\n";
        return 1;
    }

    std::cout << "TTTest all passed\n";
    return 0;
}
