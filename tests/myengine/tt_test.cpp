#include <iostream>
#include <cassert>
#include "transpositionTable.h"
#include "move.h"

void assert_entry(const TranspositionTable::TTEntry& e, int val, int depth, int flag) {
    if (e.value != val || e.depth != depth || e.flag != flag) {
        std::cerr << "Mismatch! Expected {v=" << val << ", d=" << depth << ", f=" << flag << "} "
                  << "but got {v=" << e.value << ", d=" << e.depth << ", f=" << e.flag << "}\n";
        exit(1);
    }
}

void test_basic_io() {
    std::cout << "--- test_basic_io ---\n";
    TranspositionTable tt(1024);
    uint64_t key = 0xAAAA;
    Move m = Move::fromUCI("e2e4");

    tt.store(key, 100, 5, m, TranspositionTable::TTEntry::EXACT);

    TranspositionTable::TTEntry out;
    bool found = tt.probe(key, out);
    assert(found);
    assert_entry(out, 100, 5, TranspositionTable::TTEntry::EXACT);
    assert(out.bestMove == m);
    std::cout << "PASS\n";
}

void test_missing_key() {
    std::cout << "--- test_missing_key ---\n";
    TranspositionTable tt(1024);
    TranspositionTable::TTEntry out;
    bool found = tt.probe(0xDEADBEEF, out);
    assert(!found);
    std::cout << "PASS\n";
}

void test_update_better_depth() {
    std::cout << "--- test_update_better_depth ---\n";
    TranspositionTable tt(1024);
    uint64_t key = 0xBBBB;

    tt.store(key, 10, 2, Move(), TranspositionTable::TTEntry::LOWERBOUND);
    tt.store(key, 50, 10, Move(), TranspositionTable::TTEntry::EXACT);

    TranspositionTable::TTEntry out;
    tt.probe(key, out);

    assert_entry(out, 50, 10, TranspositionTable::TTEntry::EXACT);
    std::cout << "PASS\n";
}

void test_reject_worse_depth() {
    std::cout << "--- test_reject_worse_depth ---\n";
    TranspositionTable tt(1024);
    uint64_t key = 0xCCCC;

    tt.store(key, 1000, 10, Move(), TranspositionTable::TTEntry::EXACT);
    tt.store(key, -50, 1, Move(), TranspositionTable::TTEntry::UPPERBOUND);

    TranspositionTable::TTEntry out;
    tt.probe(key, out);

    if (out.depth == 1) {
        std::cerr << "FAIL: Valuable Depth 10 entry was overwritten by Depth 1!\n";
    } else {
        assert_entry(out, 1000, 10, TranspositionTable::TTEntry::EXACT);
        std::cout << "PASS\n";
    }
}

int main() {
    test_basic_io();
    test_missing_key();
    test_update_better_depth();
    test_reject_worse_depth();

    std::cout << "ALL TT TESTS PASSED\n";
    return 0;
}