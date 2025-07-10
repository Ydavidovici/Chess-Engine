cmake_minimum_required(VERSION 3.14)
project(chess_engine)

# Allow disabling Python bindings
option(BUILD_PYTHON_BINDINGS "Build Python (pybind11) bindings" OFF)

# If bindings enabled, locate pybind11
if(BUILD_PYTHON_BINDINGS)
    execute_process(
            COMMAND python3 -m pybind11 --cmakedir
            OUTPUT_VARIABLE _PYBIND11_CMAKE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    list(APPEND CMAKE_PREFIX_PATH "${_PYBIND11_CMAKE_DIR}")
    find_package(pybind11 REQUIRED)
endif()

# Build all targets as PIC so we can link into a shared module
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#--------------------------------------------------------------------------------
#  Static libraries for each component
#--------------------------------------------------------------------------------
add_library(board     STATIC src/board.cpp)
add_library(move      STATIC src/move.cpp)
add_library(evaluator STATIC src/evaluator.cpp)
# evaluator needs Move’s definitions (and board if it uses anything from there)
target_link_libraries(evaluator
        PUBLIC
        move
        board
)
add_library(tt        STATIC src/transpositionTable.cpp)

target_include_directories(board     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(move      PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(evaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tt        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
#--------------------------------------------------------------------------------
#  Search component
#--------------------------------------------------------------------------------
add_library(search STATIC src/search.cpp src/timeman.cpp)
target_include_directories(search
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# search needs evaluator and tt (and transitively move/board)
target_link_libraries(search
        PUBLIC
        evaluator
        tt
)



#--------------------------------------------------------------------------------
#  Core engine library
#--------------------------------------------------------------------------------
add_library(core_engine STATIC src/main.cpp)
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(core_engine
        PUBLIC board move evaluator tt search
)

#--------------------------------------------------------------------------------
#  Python module (optional)
#--------------------------------------------------------------------------------
if(BUILD_PYTHON_BINDINGS)
    pybind11_add_module(pyengine src/bindings.cpp)
    target_link_libraries(pyengine PRIVATE core_engine)
    install(TARGETS pyengine LIBRARY DESTINATION .)
endif()

#--------------------------------------------------------------------------------
#  Stand-alone test binaries (one per .cpp)
#--------------------------------------------------------------------------------
add_executable(test_board_move
        ${CMAKE_CURRENT_SOURCE_DIR}/../tests/engine/board_move_tests.cpp
)
add_executable(test_eval
        ${CMAKE_CURRENT_SOURCE_DIR}/../tests/engine/eval_test.cpp
)
add_executable(test_search
        ${CMAKE_CURRENT_SOURCE_DIR}/../tests/engine/search_test.cpp
)
add_executable(test_tt
        ${CMAKE_CURRENT_SOURCE_DIR}/../tests/engine/tt_test.cpp
)

foreach(test_exe IN ITEMS test_board_move test_eval test_search test_tt)
    target_include_directories(${test_exe}
            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    # core_engine already PUBLICly pulls in board, move, evaluator, tt
    target_link_libraries(${test_exe}
            PRIVATE
            core_engine
    )
endforeach()

#--------------------------------------------------------------------------------
#  Enable CTest
#--------------------------------------------------------------------------------
enable_testing()
add_test(NAME BoardMoveTests COMMAND test_board_move)
add_test(NAME EvalTests      COMMAND test_eval)
add_test(NAME SearchTests    COMMAND test_search)
add_test(NAME TTTests        COMMAND test_tt)

#--------------------------------------------------------------------------------
#  AddressSanitizer for test_board_move only in Debug builds
#--------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_board_move PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
    )
    target_link_options(test_board_move PRIVATE -fsanitize=address)
endif()