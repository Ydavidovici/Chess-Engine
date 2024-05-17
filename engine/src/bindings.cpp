// src/bindings.cpp

#include <pybind11/pybind11.h>
#include "board.h"

namespace py = pybind11;

PYBIND11_MODULE(engine, m) {
py::class_<Board>(m, "Board")
.def(py::init<>())
.def("initializeBoard", &Board::initializeBoard);
// Bind other methods as needed
}
