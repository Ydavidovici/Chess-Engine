// backend/engine/src/bindings.cpp

#include <pybind11/pybind11.h>
#include "engine.h"

namespace py = pybind11;

PYBIND11_MODULE(engine, m) {
py::class_<Board>(m, "Board")
.def(py::init<>())
.def("initializeBoard", &Board::initializeBoard)
.def("getBoardState", &Board::getBoardState);

py::class_<Evaluator>(m, "Evaluator")
.def(py::init<>())
.def("evaluate", &Evaluator::evaluate);

py::class_<Player>(m, "Player")
.def(py::init<Player::Color>())
.def("getColor", &Player::getColor);

py::class_<Move>(m, "Move")
.def(py::init<const std::string&>())
.def("getMove", &Move::getMove);

py::class_<Engine>(m, "Engine")
.def(py::init<>())
.def("initialize", &Engine::initialize)
.def("makeMove", &Engine::makeMove)
.def("getBoardState", &Engine::getBoardState)
.def("evaluateBoard", &Engine::evaluateBoard);
}
