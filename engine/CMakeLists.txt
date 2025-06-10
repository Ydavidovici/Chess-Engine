cmake_minimum_required(VERSION 3.14)
project(chess_engine)

# Build all targets as PIC so we can link them into a shared module
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#--------------------------------------------------------------------------------
#  Find pybind11
#--------------------------------------------------------------------------------
find_package(pybind11 REQUIRED)

#--------------------------------------------------------------------------------
#  Static libraries for each component
#--------------------------------------------------------------------------------
add_library(board     STATIC src/board.cpp)
add_library(move      STATIC src/move.cpp)
add_library(evaluator STATIC src/evaluator.cpp)

target_include_directories(board     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(move      PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(evaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#--------------------------------------------------------------------------------
#  Core engine library
#--------------------------------------------------------------------------------
add_library(core_engine STATIC src/engine.cpp)
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(core_engine
        PUBLIC board move evaluator
)

#--------------------------------------------------------------------------------
#  Python module
#--------------------------------------------------------------------------------
pybind11_add_module(pyengine src/engine_pybind.cpp)
target_link_libraries(pyengine PRIVATE core_engine)

install(TARGETS pyengine
        LIBRARY DESTINATION .
)

#--------------------------------------------------------------------------------
#  Stand‑alone test binary
#--------------------------------------------------------------------------------
add_executable(test_board_move ../tests/engine/board_move_tests.cpp)
target_include_directories(test_board_move PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_board_move PRIVATE core_engine)

#--------------------------------------------------------------------------------
#  AddressSanitizer for test_engine only in Debug builds
#--------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_board_move PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
    )
    target_link_options(test_board_move PRIVATE -fsanitize=address)
endif()

enable_testing()
add_test(NAME BoardMoveTests COMMAND test_board_move)