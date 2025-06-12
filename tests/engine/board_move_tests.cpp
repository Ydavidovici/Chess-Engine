// tests/engine/board_move_tests.cpp

#include "board.h"
#include "move.h"
#include "types.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using std::vector;
using std::string;

// helper: does vec contain the string s?
static bool contains(const vector<string>& vec, const string& s) {
    return std::find(vec.begin(), vec.end(), s) != vec.end();
}

// 1) Move tests
void testMoveBasics() {
    // valid move, normal
    Move m1(0, 1, MoveType::NORMAL);
    assert(m1.isValid());
    assert(!m1.isCapture());
    assert(m1.toString() == "a1b1");

    // capture move
    Move m2(10, 17, MoveType::CAPTURE);
    assert(m2.isValid());
    assert(m2.isCapture());
    assert(m2.toString() == "c2b3");

    // en passant flagged as capture
    Move ep(20, 27, MoveType::EN_PASSANT);
    assert(ep.isValid());
    assert(ep.isCapture());

    // invalid coords
    Move bad1(-1, 5, MoveType::NORMAL);
    assert(!bad1.isValid());
    Move bad2(0, 64, MoveType::NORMAL);
    assert(!bad2.isValid());

    // invalid type
    Move bad3(0, 1, MoveType::INVALID);
    assert(!bad3.isValid());

    // promotion suffix
    Move prom(6, 14, MoveType::PROMOTION, 'Q');
    assert(prom.isValid());
    assert(prom.toString() == "g1g2Q");
}

// 2) Board inspection tests
void testInitialFEN() {
    Board b;
    string start = b.toFEN();
    const string expected =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assert(start == expected);
}

void testFENRoundTrip() {
    const string F = "2kr3r/ppp2ppp/8/8/8/8/PPP2PPP/2KR3R b Qk - 5 42";
    Board b;
    b.loadFEN(F);
    assert(b.toFEN() == F);
}

// gather all pseudo-moves as SAN-like strings
static vector<string> gen(const Board& b) {
    auto mv = b.generateLegalMoves();
    vector<string> out; out.reserve(mv.size());
    for (auto &m : mv) out.push_back(m.toString());
    return out;
}

// 3) Pseudo-legal move generation
void testPawnPushes() {
    Board b;
    b.loadFEN("8/8/8/8/8/8/4P3/8 w - - 0 1"); // pawn on e2
    auto v = gen(b);
    assert(contains(v, "e2e3"));
    assert(contains(v, "e2e4"));

    b.loadFEN("8/8/8/8/4P3/8/8/8 w - - 0 1"); // pawn on e5
    v = gen(b);
    assert(contains(v, "e5e6"));
    assert(!contains(v, "e5e7"));  // double not allowed
}

void testPawnPromotions() {
    Board b;
    b.loadFEN("8/P7/8/8/8/8/8/8 w - - 0 1"); // pawn on a7
    auto v = gen(b);
    // should have all four promotion flavors to a8
    assert(contains(v, "a7a8Q"));
    assert(contains(v, "a7a8R"));
    assert(contains(v, "a7a8B"));
    assert(contains(v, "a7a8N"));
}

void testKnightMoves() {
    Board b;
    b.loadFEN("8/8/8/8/8/8/8/N7 w - - 0 1"); // knight on a1
    auto v = gen(b);
    // from a1 can go to b3 and c2
    assert(contains(v, "a1b3"));
    assert(contains(v, "a1c2"));
}

void testSlidingMoves() {
    // Rook on d4
    Board b;
    b.loadFEN("8/8/8/3R4/8/8/8/8 w - - 0 1");
    auto v = gen(b);
    // should include d4d8, d4d1, d4a4, d4h4
    assert(contains(v, "d4d8"));
    assert(contains(v, "d4d1"));
    assert(contains(v, "d4a4"));
    assert(contains(v, "d4h4"));

    // Bishop on d4
    b.loadFEN("8/8/8/3B4/8/8/8/8 w - - 0 1");
    v = gen(b);
    assert(contains(v, "d4a7"));
    assert(contains(v, "d4g7"));
    assert(contains(v, "d4a1"));
    assert(contains(v, "d4g1"));

    // Queen on d4
    b.loadFEN("8/8/8/3Q4/8/8/8/8 w - - 0 1");
    v = gen(b);
    // queen = rook + bishop
    assert(contains(v, "d4d8"));
    assert(contains(v, "d4a7"));
}

void testKingMoves() {
    Board b;
    b.loadFEN("8/8/8/4K3/8/8/8/8 w - - 0 1"); // king on e4
    auto v = gen(b);
    // e4 -> d5,e5,f5,d4,f4,d3,e3,f3
    for (auto sq : {"d5","e5","f5","d4","f4","d3","e3","f3"}) {
        string m = string("e4") + sq;
        assert(contains(v, m));
    }
}

void testCastlingGeneration() {
    // both sides with full rights, empty between
    Board b;
    b.loadFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    auto v = gen(b);
    // white should see e1g1 and e1c1
    assert(contains(v, "e1g1") && "white kingside castling missing");
    assert(contains(v, "e1c1") && "white queenside castling missing");

    // now black to move
    b.loadFEN("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    v = gen(b);
    assert(contains(v, "e8g8") && "black kingside castling missing");
    assert(contains(v, "e8c8") && "black queenside castling missing");
}

void testEnPassantGeneration() {
    // White to move, pawn on e5, black pawn on d5, ep at d6
    Board b;
    b.loadFEN("8/8/8/3pP3/8/8/8/8 w - d6 0 1");
    auto v = gen(b);
    // e5 can capture d6 en passant
    bool found = false;
    for (auto &s : v) {
        if (s == "e5d6") {
            // also check MoveType
            for (auto &m : b.generateLegalMoves()) {
                if (m.toString() == "e5d6") {
                    assert(m.type == MoveType::EN_PASSANT);
                    found = true;
                }
            }
        }
    }
    assert(found && "en passant move missing");
}

// 4) make/unmake spot checks
void testMakeUnmake() {
    Board b;
    // simple pawn push & undo
    b.loadFEN("8/8/8/8/8/8/4P3/8 w - - 0 1");
    Move m(28, 36);    // e2->e3
    assert(b.makeMove(m));
    assert(b.pieceBB(Color::WHITE, Board::PAWN) & (1ULL<<36));
    b.unmakeMove();
    assert(b.pieceBB(Color::WHITE, Board::PAWN) & (1ULL<<28));

    // castle undo
    b.loadFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    Move ck(4, 6, MoveType::CASTLE_KINGSIDE);
    assert(b.makeMove(ck));
    // rook should have moved
    assert(b.pieceBB(Color::WHITE, Board::ROOK) & (1ULL<<5));
    b.unmakeMove();
    // back to e1,a1 and h1
    assert(b.pieceBB(Color::WHITE, Board::KING) & (1ULL<<4));
    assert(b.pieceBB(Color::WHITE, Board::ROOK) & (1ULL<<0));
}

int main() {
    testMoveBasics();
    testInitialFEN();
    testFENRoundTrip();

    testPawnPushes();
    testPawnPromotions();
    testKnightMoves();
    testSlidingMoves();
    testKingMoves();
    testCastlingGeneration();
    testEnPassantGeneration();

    testMakeUnmake();

    std::cout << "ALL TESTS PASSED\n";
    return 0;
}
