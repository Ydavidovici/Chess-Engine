cmake_minimum_required(VERSION 3.14)
project(chess_engine)

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
add_library(board     STATIC board.cpp)
add_library(move      STATIC move.cpp)
add_library(evaluator STATIC evaluator.cpp)

target_include_directories(board     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(move      PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(evaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------------------
#  Core engine library
#--------------------------------------------------------------------------------
add_library(core_engine STATIC engine.cpp)
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(core_engine
        PUBLIC board move evaluator
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
target_link_libraries(test_engine PRIVATE core_engine)

#--------------------------------------------------------------------------------
#  AddressSanitizer for test_engine only in Debug builds
#--------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_engine PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
    )
    target_link_options(test_engine PRIVATE -fsanitize=address)
endif()