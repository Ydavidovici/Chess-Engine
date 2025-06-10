// tests/engine/board_move_tests.cpp
#include "board.h"
#include "move.h"
#include <cassert>
#include <iostream>

using namespace std;

void testBoardInitialFEN() {
    Board b;
    string startFEN = b.toFEN();
    string expected = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assert(startFEN == expected && "Initial FEN mismatch");
}

void testFENRoundTrip() {
    Board b;
    string fen = "8/PP6/8/8/8/8/pp6/8 b Qk - 5 10";
    b.loadFEN(fen);
    assert(b.toFEN() == fen && "FEN round-trip failed");
}

void testCastlingRights() {
    Board b;
    // White kingside
    b.loadFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    auto moves = b.generateLegalMoves();
    bool canCastle = false;
    for (auto &m : moves) {
        if (m.type == MoveType::CASTLE_KINGSIDE && m.start == 4) canCastle = true;
    }
    assert(canCastle && "White kingside castling missing");
}

void testEnPassant() {
    Board b;
    // Set EP target
    b.loadFEN("8/8/8/3pP3/8/8/8/8 w - d6 0 1");
    auto moves = b.generatePseudoMoves();
    bool hasEP = false;
    for (auto &m : moves) {
        if (m.type == MoveType::EN_PASSANT && m.end == (3*8 + 3)) {
            hasEP = true;
            b.makeMove(m);
            // After EP, pawn should be at d6 and captured pawn removed
            string fen = b.toFEN();
            assert(fen.find("3P4") != string::npos && "En passant capture failed in FEN");
            break;
        }
    }
    assert(hasEP && "En passant move not generated");
}

void testPromotion() {
    Board b;
    // White pawn promotion
    b.loadFEN("8/P7/8/8/8/8/8/8 w - - 0 1");
    auto moves = b.generatePseudoMoves();
    bool gotPromo = false;
    for (auto &m : moves) {
        if (m.type == MoveType::PROMOTION && m.promo == 'Q') {
            gotPromo = true;
            b.makeMove(m);
            string fen = b.toFEN();
            assert(fen.find("Q7") != string::npos && "Promotion to queen failed");
            break;
        }
    }
    assert(gotPromo && "Promotion move not generated");
}

void testMoveToString() {
    Move m(12, 28, MoveType::NORMAL);
    assert(m.toString() == "e2e4" && "Move toString normal failed");
    Move m2(6, 0, MoveType::PROMOTION, 'Q');
    assert(m2.toString() == "g1a1Q" && "Move toString promotion failed");
}

int main() {
    testBoardInitialFEN();
    testFENRoundTrip();
    testCastlingRights();
    testEnPassant();
    testPromotion();
    testMoveToString();
    cout << "All board and move tests passed!" << endl;
    return 0;
}
