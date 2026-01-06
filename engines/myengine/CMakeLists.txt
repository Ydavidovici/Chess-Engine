cmake_minimum_required(VERSION 3.14)
project(chess_engine)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# -------------------------------------------------------------------
# Core libraries
# -------------------------------------------------------------------
add_library(board STATIC src/board.cpp)
add_library(move STATIC src/move.cpp)
add_library(evaluator STATIC src/evaluator.cpp)
add_library(tt STATIC src/transpositionTable.cpp)
add_library(search STATIC src/search.cpp src/timeManager.cpp)

add_library(core_engine STATIC src/engine.cpp)

target_include_directories(board PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(move PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(evaluator PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(tt PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(search PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(core_engine PUBLIC ${ENGINE_INCLUDE_DIR})

target_link_libraries(evaluator PUBLIC move board)
target_link_libraries(search PUBLIC evaluator tt board move)
target_link_libraries(core_engine PUBLIC board move evaluator tt search)

# -------------------------------------------------------------------
# UCI executable
# -------------------------------------------------------------------
add_executable(myengine
    src/main.cpp
)

target_include_directories(myengine PRIVATE ${ENGINE_INCLUDE_DIR})
target_link_libraries(myengine PRIVATE core_engine)

# -------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------
enable_testing()

add_executable(test_board_move ../../tests/myengine/board_move_tests.cpp)
add_executable(test_eval ../../tests/myengine/eval_test.cpp)
add_executable(test_search ../../tests/myengine/search_test.cpp)
add_executable(test_tt ../../tests/myengine/tt_test.cpp)

foreach (test_exe IN ITEMS test_board_move test_eval test_search test_tt)
    target_include_directories(${test_exe} PRIVATE ${ENGINE_INCLUDE_DIR})
    target_link_libraries(${test_exe} PRIVATE core_engine)
endforeach ()

add_test(NAME board_move_tests COMMAND test_board_move)
set_tests_properties(board_move_tests PROPERTIES LABELS "myengine")

add_test(NAME eval_test COMMAND test_eval)
set_tests_properties(eval_test PROPERTIES LABELS "myengine")

add_test(NAME search_test COMMAND test_search)
set_tests_properties(search_test PROPERTIES LABELS "myengine")

add_test(NAME tt_test COMMAND test_tt)
set_tests_properties(tt_test PROPERTIES LABELS "myengine")
