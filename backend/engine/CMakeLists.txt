cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 14)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/backend/engine/include)

# Add library for Python bindings
find_package(pybind11 REQUIRED)

add_library(engine MODULE
        ${CMAKE_SOURCE_DIR}/backend/engine/src/bindings.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/board.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/piece.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/engine.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/evaluator.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/player.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/move.cpp
)
set_target_properties(engine PROPERTIES PREFIX "")

target_link_libraries(engine PRIVATE pybind11::module)

# Add executable for standalone testing
add_executable(ChessEngine
        ${CMAKE_SOURCE_DIR}/backend/engine/src/main.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/board.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/piece.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/engine.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/evaluator.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/player.cpp
        ${CMAKE_SOURCE_DIR}/backend/engine/src/move.cpp
)
