cmake_minimum_required(VERSION 3.12)
project(ChessEngine)

# Change this line from C++11 to C++14
set(CMAKE_CXX_STANDARD 14)  # Use C++14 standard

# Include directories for header files
include_directories(engine/include)

# Add all source files to the project
add_executable(test_engine
        engine/src/test_engine.cpp
        engine/src/board.cpp
        engine/src/engine.cpp
        engine/src/evaluator.cpp
        engine/src/move.cpp
        engine/src/piece.cpp
        engine/src/player.cpp
        engine/src/main.cpp
)

# Optionally, link libraries if needed
# target_link_libraries(test_chess_engine ...)
