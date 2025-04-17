cmake_minimum_required(VERSION 3.14)
project(chess_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

add_library(board STATIC board.cpp)
target_include_directories(board PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(move STATIC move.cpp)
target_include_directories(move PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(evaluator STATIC evaluator.cpp)
target_include_directories(evaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(core_engine STATIC engine.cpp)
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(core_engine PUBLIC board move evaluator)

pybind11_add_module(pyengine engine_pybind.cpp)
target_link_libraries(pyengine PRIVATE core_engine)

install(TARGETS pyengine
        LIBRARY DESTINATION .
)