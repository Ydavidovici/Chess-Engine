cmake_minimum_required(VERSION 3.14)
project(chess_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#--------------------------------------------------------------------------------
#  AddressSanitizer in Debug builds
#--------------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(   -fsanitize=address)
endif()

#--------------------------------------------------------------------------------
#  Find pybind11
#--------------------------------------------------------------------------------
find_package(pybind11 REQUIRED)

#--------------------------------------------------------------------------------
#  Static libraries for each component
#--------------------------------------------------------------------------------
add_library(board     STATIC board.cpp)
add_library(move      STATIC move.cpp)
add_library(evaluator STATIC evaluator.cpp)

# Make headers visible
target_include_directories(board     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(move      PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(evaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------------------
#  Core engine library
#--------------------------------------------------------------------------------
add_library(core_engine STATIC engine.cpp)
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Pull in board/move/evaluator
target_link_libraries(core_engine
        PUBLIC
        board
        move
        evaluator
)

#--------------------------------------------------------------------------------
#  Python module
#--------------------------------------------------------------------------------
pybind11_add_module(pyengine engine_pybind.cpp)
target_link_libraries(pyengine PRIVATE core_engine)

install(TARGETS pyengine
        LIBRARY DESTINATION .
)

#--------------------------------------------------------------------------------
#  Stand‑alone test binary
#--------------------------------------------------------------------------------
add_executable(test_engine test_engine.cpp)
target_include_directories(test_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# test_engine only needs core_engine (which itself brings in board/move/evaluator)
target_link_libraries(test_engine
        PRIVATE
        core_engine
)

# In Debug we’ve already added ASan flags above