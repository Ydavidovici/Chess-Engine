cmake_minimum_required(VERSION 3.14)
project(chess_engine)
add_executable(chess
  main.cpp
  move.cpp
  board.cpp
  evaluator.cpp
  engine.cpp
)
target_include_directories(chess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})