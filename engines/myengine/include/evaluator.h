#pragma once

#include "board.h"
#include <vector>
#include <cstdint>

class Evaluator {
public:
    static constexpr int PST_COUNT = 6;

    Evaluator();

    int evaluate(const Board& board, Color side_to_move) const;
    int evaluateTerminal(const Board& board, Color side_to_move) const;
    uint64_t generateZobristHash(const Board& board, bool sideToMove) const;

private:
    int evaluateMaterial(const Board& board) const;
    int evaluatePositional(const Board& board) const;

    const int pieceValues[PST_COUNT] = {100, 320, 330, 500, 900, 20000};

    std::vector<int> whitePawnTable;
    std::vector<int> whiteKnightTable;
    std::vector<int> whiteBishopTable;
    std::vector<int> whiteRookTable;
    std::vector<int> whiteQueenTable;
    std::vector<int> whiteKingTableMG;
    std::vector<int> whiteKingTableEG;

    std::vector<int> blackPawnTable;
    std::vector<int> blackKnightTable;
    std::vector<int> blackBishopTable;
    std::vector<int> blackRookTable;
    std::vector<int> blackQueenTable;
    std::vector<int> blackKingTableMG;
    std::vector<int> blackKingTableEG;

    std::vector<std::vector<uint64_t>> zobristKeys;
    uint64_t zobristSide;

    void initializePieceSquareTables();
};
