// move.h
#pragma once
#include <optional>
#include <string>
#include "types.h"

struct Move {
  int start, end;
  MoveType type = MoveType::NORMAL;
  std::optional<char> promo;  // 'Q','R','B','N'

  bool isValid() const;
  bool isCapture() const;
  std::string toString() const;
};