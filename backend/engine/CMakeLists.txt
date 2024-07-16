cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

include_directories(${CMAKE_SOURCE_DIR}/backend/engine/include)

set(SOURCES
        src/bindings.cpp
        src/board.cpp
        src/engine.cpp
        src/evaluator.cpp
        src/move.cpp
        src/piece.cpp
        src/player.cpp
)

# Find Python
find_package(Python3 COMPONENTS Development REQUIRED)

# Create the shared library for the Python module
add_library(chess_engine MODULE ${SOURCES})
set_target_properties(chess_engine PROPERTIES PREFIX "" SUFFIX ".so")

# Link against Python libraries
target_link_libraries(chess_engine PRIVATE Python3::Python)
