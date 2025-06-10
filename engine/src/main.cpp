#include "engine.h"
#include <iostream>

int main(){
  Engine e;
  e.newGame();
  for(auto& row : e.getBoardState())
    std::cout << row << "\n";
  // … parse moves from stdin, call e.makeMove(), e.findBestMove(), etc.
  return 0;
}