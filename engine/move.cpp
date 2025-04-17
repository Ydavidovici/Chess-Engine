// move.cpp
#include "move.h"
#include <cassert>

bool Move::isValid() const {
  return start>=0 && start<64 && end>=0 && end<64 && type!=MoveType::INVALID;
}

bool Move::isCapture() const {
  return type==MoveType::CAPTURE || type==MoveType::EN_PASSANT;
}

std::string Move::toString() const {
  auto f = [](int sq){
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string{file, rank};
  };
  std::string s = f(start) + f(end);
  if (promo) s += *promo;
  return s;
}