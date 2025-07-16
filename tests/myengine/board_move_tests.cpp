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

// helper: dump the generated moves for a fen
static void dumpMoves(const Board& b, const std::string& fen) {
    auto mv = b.generateLegalMoves();
    std::vector<std::string> out; out.reserve(mv.size());
    for (auto &m : mv) out.push_back(m.toString());
    std::cout << "FEN = \"" << fen << "\"\n"
              << "  Moves (" << out.size() << "): ";
    for (auto &s : out) std::cout << s << "  ";
    std::cout << "\n";
}

// 1) Move tests
void testMoveBasics() {
    std::cout << "--- testMoveBasics ---\n";
    Move m1(0, 1, MoveType::NORMAL);
    assert(m1.isValid());       std::cout<<"  ok normal-valid\n";
    assert(!m1.isCapture());    std::cout<<"  ok normal-nonCapture\n";
    assert(m1.toString()=="a1b1"); std::cout<<"  ok toString a1b1\n";

    Move m2(10,17,MoveType::CAPTURE);
    assert(m2.isCapture());     std::cout<<"  ok capture\n";
    assert(m2.toString()=="c2b3"); std::cout<<"  ok toString c2b3\n";

    Move ep(20,27,MoveType::EN_PASSANT);
    assert(ep.isValid());       std::cout<<"  ok ep-valid\n";
    assert(ep.isCapture());     std::cout<<"  ok ep-isCapture\n";

    Move bad1(-1,5,MoveType::NORMAL);
    assert(!bad1.isValid());    std::cout<<"  ok bad1-invalid\n";
    Move bad2(0,64,MoveType::NORMAL);
    assert(!bad2.isValid());    std::cout<<"  ok bad2-invalid\n";
    Move bad3(0,1,MoveType::INVALID);
    assert(!bad3.isValid());    std::cout<<"  ok bad3-invalidType\n";

    Move prom(6,14,MoveType::PROMOTION,'Q');
    assert(prom.isValid());     std::cout<<"  ok promo-valid\n";
    assert(prom.toString()=="g1g2Q"); std::cout<<"  ok promo-toString\n\n";
}

// 2) Board inspection tests
void testInitialFEN() {
    std::cout << "--- testInitialFEN ---\n";
    Board b;
    auto s = b.toFEN();
    std::cout<<"  toFEN: "<<s<<"\n";
    const string exp="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assert(s==exp);
    std::cout<<"  ok initial FEN\n\n";
}

void testFENRoundTrip() {
    std::cout << "--- testFENRoundTrip ---\n";
    const string F="2kr3r/ppp2ppp/8/8/8/8/PPP2PPP/2KR3R b Qk - 5 42";
    Board b; b.loadFEN(F);
    auto s=b.toFEN();
    std::cout<<"  round-trip: "<<s<<"\n";
    assert(s==F);
    std::cout<<"  ok FEN round-trip\n\n";
}

// generate & dump helper
static vector<string> gen(const Board& b) {
    auto mv = b.generateLegalMoves();
    vector<string> out; out.reserve(mv.size());
    for (auto &m : mv) out.push_back(m.toString());
    return out;
}

// 3) Pseudo-legal move generation
void testPawnPushes() {
    std::cout << "--- testPawnPushes ---\n";

    // pawn on e2
    {
        const std::string fen = "8/8/8/8/8/8/4P3/8 w - - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b, fen);
        auto v = gen(b);
        assert(contains(v, "e2e3"));
        assert(contains(v, "e2e4"));
    }

    // pawn on e5
    {
        const std::string fen = "8/8/8/4P3/8/8/8/8 w - - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b, fen);
        auto v = gen(b);
        assert(contains(v, "e5e6"));
        assert(!contains(v, "e5e7"));
    }

    std::cout << "  ok pawn pushes\n\n";
}

void testPawnPromotions() {
    std::cout<<"--- testPawnPromotions ---\n";
    const string fen="8/P7/8/8/8/8/8/8 w - - 0 1";
    Board b; b.loadFEN(fen);
    dumpMoves(b,fen);
    auto v = gen(b);
    assert(contains(v,"a7a8Q"));
    assert(contains(v,"a7a8R"));
    assert(contains(v,"a7a8B"));
    assert(contains(v,"a7a8N"));
    std::cout<<"  ok pawn promotions\n\n";
}

void testKnightMoves() {
    std::cout<<"--- testKnightMoves ---\n";
    const string fen="8/8/8/8/8/8/8/N7 w - - 0 1";
    Board b; b.loadFEN(fen);
    dumpMoves(b,fen);
    auto v = gen(b);
    assert(contains(v,"a1b3"));
    assert(contains(v,"a1c2"));
    std::cout<<"  ok knight moves\n\n";
}

void testSlidingMoves() {
    std::cout << "--- testSlidingMoves ---\n";

    // Rook on d4
    {
        const std::string fen = "8/8/8/8/3R4/8/8/8 w - - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b, fen);
        auto v = gen(b);
        assert(contains(v, "d4d8"));
        assert(contains(v, "d4d1"));
        assert(contains(v, "d4a4"));
        assert(contains(v, "d4h4"));
    }

    // Bishop on d4
    {
        const std::string fen = "8/8/8/8/3B4/8/8/8 w - - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b, fen);
        auto v = gen(b);
        assert(contains(v, "d4a7"));
        assert(contains(v, "d4g7"));
        assert(contains(v, "d4a1"));
        assert(contains(v, "d4g1"));
    }

    // Queen on d4
    {
        const std::string fen = "8/8/8/8/3Q4/8/8/8 w - - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b, fen);
        auto v = gen(b);
        assert(contains(v, "d4d8"));
        assert(contains(v, "d4a7"));
    }

    std::cout << "  ok sliding moves\n\n";
}

void testKingMoves() {
    std::cout<<"--- testKingMoves ---\n";
    const string fen = "8/8/8/8/4K3/8/8/8 w - - 0 1";
    Board b; b.loadFEN(fen);
    dumpMoves(b,fen);
    auto v = gen(b);
    for (auto sq:{"d5","e5","f5","d4","f4","d3","e3","f3"}) {
        string m = string("e4") + sq;
        assert(contains(v,m));
    }
    std::cout<<"  ok king moves\n\n";
}

void testCastlingGeneration() {
    std::cout<<"--- testCastlingGeneration ---\n";
    {
        const string fen="r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b,fen);
        auto v = gen(b);
        assert(contains(v,"e1g1"));
        assert(contains(v,"e1c1"));
    }
    {
        const string fen="r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1";
        Board b; b.loadFEN(fen);
        dumpMoves(b,fen);
        auto v = gen(b);
        assert(contains(v,"e8g8"));
        assert(contains(v,"e8c8"));
    }
    std::cout<<"  ok castling\n\n";
}

void testEnPassantGeneration() {
    std::cout<<"--- testEnPassantGeneration ---\n";
    const string fen="8/8/8/3pP3/8/8/8/8 w - d6 0 1";
    Board b; b.loadFEN(fen);
    dumpMoves(b,fen);
    auto mv = b.generateLegalMoves();
    bool found = false;
    for (auto &m : mv) {
        if (m.toString()=="e5d6" && m.type==MoveType::EN_PASSANT)
            found = true;
    }
    assert(found);
    std::cout<<"  ok en-passant\n\n";
}

// 4) Illegal-move filtering
void testIllegalKingMoves() {
    std::cout<<"--- testIllegalKingMoves ---\n";
    // King on e1 attacked from f2 by a black rook
    const string fen = "8/8/8/8/8/8/5r2/4K3 w - - 0 1";
    Board b; b.loadFEN(fen);
    dumpMoves(b,fen);
    auto v = gen(b);
    // e1-f1 is attacked → must NOT appear
    assert(!contains(v,"e1f1"));
    // e1-d1 is safe → should appear
    assert(contains(v,"e1d1"));
    std::cout<<"  ok illegal king moves filtered\n\n";
}

void testMakeUnmake() {
    std::cout<<"--- testMakeUnmake ---\n";
    {
        const string fen="8/8/8/8/8/8/4P3/8 w - - 0 1";
        Board b; b.loadFEN(fen);
        Move m(28,36);
        assert(b.makeMove(m));
        assert(b.pieceBB(Color::WHITE,Board::PAWN)&(1ULL<<36));
        b.unmakeMove();
        assert(b.pieceBB(Color::WHITE,Board::PAWN)&(1ULL<<28));
    }
    {
        const string fen="r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
        Board b; b.loadFEN(fen);
        Move ck(4,6,MoveType::CASTLE_KINGSIDE);
        assert(b.makeMove(ck));
        assert(b.pieceBB(Color::WHITE,Board::ROOK)&(1ULL<<5));
        b.unmakeMove();
        assert(b.pieceBB(Color::WHITE,Board::KING)&(1ULL<<4));
        assert(b.pieceBB(Color::WHITE,Board::ROOK)&(1ULL<<0));
    }
    std::cout<<"  ok make/unmake\n\n";
}

int main(){
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

    testIllegalKingMoves();

    testMakeUnmake();

    std::cout<<"ALL TESTS PASSED\n";
    return 0;
}
