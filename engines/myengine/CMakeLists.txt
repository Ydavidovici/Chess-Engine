cmake_minimum_required(VERSION 3.14)
project(chess_engine)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# -------------------------------------------------------------------
# Core libraries (no pybind11 headers here)
# -------------------------------------------------------------------
add_library(board        STATIC src/board.cpp)
add_library(move         STATIC src/move.cpp)
add_library(evaluator    STATIC src/evaluator.cpp)
add_library(tt           STATIC src/transpositionTable.cpp)
add_library(search       STATIC src/search.cpp src/timeman.cpp)
add_library(core_engine  STATIC src/main.cpp)

target_include_directories(board        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(move         PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(evaluator    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tt           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(search       PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(core_engine  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(evaluator     PUBLIC move board)
target_link_libraries(search        PUBLIC evaluator tt board move)
target_link_libraries(core_engine   PUBLIC board move evaluator tt search)

# -------------------------------------------------------------------
# Python bindings
# -------------------------------------------------------------------
option(BUILD_PYTHON_BINDINGS "Build Python (pybind11) bindings" ON)
set(PYBIND11_FINDPYTHON ON)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

if(BUILD_PYTHON_BINDINGS)
    # Build the Python module as 'chessengine', matching the PYBIND11_MODULE name.
    pybind11_add_module(chessengine src/bindings.cpp)
    target_link_libraries(chessengine PRIVATE core_engine)
    install(TARGETS chessengine LIBRARY DESTINATION .)
endif()

# -------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------
enable_testing()

add_executable(test_board_move   ../../tests/myengine/board_move_tests.cpp)
add_executable(test_eval         ../../tests/myengine/eval_test.cpp)
add_executable(test_search       ../../tests/myengine/search_test.cpp)
add_executable(test_tt           ../../tests/myengine/tt_test.cpp)

foreach(test_exe IN ITEMS test_board_move test_eval test_search test_tt)
    target_include_directories(${test_exe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${test_exe} PRIVATE core_engine)
endforeach()

# Name the CTest tests to match file stems and add a label
add_test(NAME board_move_tests COMMAND test_board_move)
set_tests_properties(board_move_tests PROPERTIES LABELS "myengine")

add_test(NAME eval_test COMMAND test_eval)
set_tests_properties(eval_test PROPERTIES LABELS "myengine")

add_test(NAME search_test COMMAND test_search)
set_tests_properties(search_test PROPERTIES LABELS "myengine")

add_test(NAME tt_test COMMAND test_tt)
set_tests_properties(tt_test PROPERTIES LABELS "myengine")
