cmake_minimum_required(VERSION 3.10)
project(Chess-Engine)

set(CMAKE_CXX_STANDARD 14)

find_package(pybind11 REQUIRED)

include_directories(include)
add_library(engine MODULE src/bindings.cpp src/board.cpp)

target_link_libraries(engine PRIVATE pybind11::module)
