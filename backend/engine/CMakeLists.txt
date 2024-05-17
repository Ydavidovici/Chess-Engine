cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 14)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add library for Python bindings
find_package(pybind11 REQUIRED)

add_library(engine MODULE
        ${CMAKE_SOURCE_DIR}/src/bindings.cpp
        ${CMAKE_SOURCE_DIR}/src/board.cpp
        ${CMAKE_SOURCE_DIR}/src/piece.cpp
        ${CMAKE_SOURCE_DIR}/src/engine.cpp
        ${CMAKE_SOURCE_DIR}/src/evaluator.cpp
        ${CMAKE_SOURCE_DIR}/src/player.cpp
        ${CMAKE_SOURCE_DIR}/src/move.cpp
)
target_link_libraries(engine PRIVATE pybind11::module)

# Add executable for standalone testing
add_executable(ChessEngine
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/board.cpp
        ${CMAKE_SOURCE_DIR}/src/piece.cpp
        ${CMAKE_SOURCE_DIR}/src/engine.cpp
        ${CMAKE_SOURCE_DIR}/src/evaluator.cpp
        ${CMAKE_SOURCE_DIR}/src/player.cpp
        ${CMAKE_SOURCE_DIR}/src/move.cpp
)
