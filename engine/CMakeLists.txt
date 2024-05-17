cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 14)

# Include directories
include_directories(include)

# Add library for Python bindings
find_package(pybind11 REQUIRED)

add_library(engine MODULE
        src/bindings.cpp
        src/board.cpp
        src/piece.cpp
        src/engine.cpp
        src/evaluator.cpp
        src/player.cpp
        src/move.cpp
)
target_link_libraries(engine PRIVATE pybind11::module)

# Add executable for standalone testing
add_executable(ChessEngine
        src/main.cpp
        src/board.cpp
        src/piece.cpp
        src/engine.cpp
        src/evaluator.cpp
        src/player.cpp
        src/move.cpp
)
