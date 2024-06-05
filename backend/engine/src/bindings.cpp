#include "board.h"
#include "engine.h"
#include "move.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(engine, m) {
py::class_<Board>(m, "Board")
.def(py::init<>())
.def("initializeBoard", &Board::initializeBoard)
.def("printBoard", &Board::printBoard)
.def("getBoardState", &Board::getBoardState);  // Add this method if it's defined

py::class_<Move>(m, "Move")
.def(py::init<const std::string &>())
.def("getMove", &Move::getMove);

py::class_<Engine>(m, "Engine")
.def(py::init<>())
.def("initialize", &Engine::initialize)
.def("makeMove", &Engine::makeMove)
.def("getBoardState", &Engine::getBoardState)
.def("evaluateBoard", &Engine::evaluateBoard);
}
