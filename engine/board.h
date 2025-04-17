// board.h
#pragma once
#include <vector>
#include <string>
#include "move.h"

class Board {
public:
  Board();
  void initialize();
  void loadFEN(const std::string& fen);
  std::vector<std::string> getBoardState() const;
  bool makeMove(const Move& m, Color c);
  std::vector<Move> generateLegalMoves(Color c) const;

private:
  uint64_t whitePawns, whiteKnights, whiteBishops,
           whiteRooks, whiteQueens, whiteKings;
  uint64_t blackPawns, blackKnights, blackBishops,
           blackRooks, blackQueens, blackKings;

  static int pos(int rank, int file) { return rank*8 + file; }
  bool bit(uint64_t bb, int sq) const { return (bb>>sq)&1; }
  void set(uint64_t& bb, int sq) { bb |= (1ULL<<sq); }
  void clear(uint64_t& bb, int sq) { bb &= ~(1ULL<<sq); }
};