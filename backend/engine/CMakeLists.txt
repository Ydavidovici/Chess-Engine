# backend/engine/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

find_package(pybind11 REQUIRED)

include_directories(include)

add_library(engine SHARED
        src/bindings.cpp
        src/board.cpp
        src/engine.cpp
        src/evaluator.cpp
        src/move.cpp
        src/piece.cpp
        src/player.cpp
)

target_link_libraries(engine PRIVATE pybind11::module)
