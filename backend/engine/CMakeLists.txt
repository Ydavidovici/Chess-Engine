<<<<<<< HEAD
# backend/engine/CMakeLists.txt

=======
>>>>>>> d5b597bb403a694a8c96a4650810f9ae056749e5
cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 14)

# Include directories
<<<<<<< HEAD
include_directories(include)
=======
include_directories(${CMAKE_SOURCE_DIR}/include)
>>>>>>> d5b597bb403a694a8c96a4650810f9ae056749e5

# Add library for Python bindings
find_package(pybind11 REQUIRED)

add_library(engine MODULE
<<<<<<< HEAD
        src/bindings.cpp
        src/board.cpp
        src/piece.cpp
        src/engine.cpp
        src/evaluator.cpp
        src/player.cpp
        src/move.cpp
=======
        ${CMAKE_SOURCE_DIR}/src/bindings.cpp
        ${CMAKE_SOURCE_DIR}/src/board.cpp
        ${CMAKE_SOURCE_DIR}/src/piece.cpp
        ${CMAKE_SOURCE_DIR}/src/engine.cpp
        ${CMAKE_SOURCE_DIR}/src/evaluator.cpp
        ${CMAKE_SOURCE_DIR}/src/player.cpp
        ${CMAKE_SOURCE_DIR}/src/move.cpp
>>>>>>> d5b597bb403a694a8c96a4650810f9ae056749e5
)
target_link_libraries(engine PRIVATE pybind11::module)

# Add executable for standalone testing
add_executable(ChessEngine
<<<<<<< HEAD
        src/main.cpp
        src/board.cpp
        src/piece.cpp
        src/engine.cpp
        src/evaluator.cpp
        src/player.cpp
        src/move.cpp
=======
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/board.cpp
        ${CMAKE_SOURCE_DIR}/src/piece.cpp
        ${CMAKE_SOURCE_DIR}/src/engine.cpp
        ${CMAKE_SOURCE_DIR}/src/evaluator.cpp
        ${CMAKE_SOURCE_DIR}/src/player.cpp
        ${CMAKE_SOURCE_DIR}/src/move.cpp
>>>>>>> d5b597bb403a694a8c96a4650810f9ae056749e5
)
