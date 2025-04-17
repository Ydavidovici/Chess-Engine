#include <pybind11/pybind11.h>
#include <pybind11/stl.h>   // for std::vector<std::string>
#include "engine.h"
#include "move.h"
#include "types.h"

namespace py = pybind11;

PYBIND11_MODULE(pyengine, m) {
m.doc() = "Chess engine bindings";

// Color enum
py::enum_<Color>(m, "Color")
.value("WHITE", Color::WHITE)
.value("BLACK", Color::BLACK)
.export_values();

// MoveType enum
py::enum_<MoveType>(m, "MoveType")
.value("NORMAL",           MoveType::NORMAL)
.value("CAPTURE",          MoveType::CAPTURE)
.value("CASTLE_KINGSIDE",  MoveType::CASTLE_KINGSIDE)
.value("CASTLE_QUEENSIDE", MoveType::CASTLE_QUEENSIDE)
.value("PROMOTION",        MoveType::PROMOTION)
.value("EN_PASSANT",       MoveType::EN_PASSANT)
.value("INVALID",          MoveType::INVALID)
.export_values();

// Move struct
py::class_<Move>(m, "Move")
.def(py::init<>())                           // default ctor
.def_readwrite("start", &Move::start)
.def_readwrite("end",   &Move::end)
.def_readwrite("type",  &Move::type)
.def_readwrite("promo", &Move::promo)
.def("is_valid",   &Move::isValid)
.def("is_capture", &Move::isCapture)
.def("to_string",  &Move::toString);

// Engine class
py::class_<Engine>(m, "Engine")
.def(py::init<>())
// reset to starting position, returns void
.def("new_game",   &Engine::newGame)
// print board to stdout
.def("print_board",&Engine::printBoard)
// apply a move
.def("make_move",  &Engine::makeMove, py::arg("move"), py::arg("color"))
// static eval
.def("evaluate_board", &Engine::evaluateBoard)
// best move search
.def("find_best_move",
&Engine::findBestMove,
py::arg("max_depth"), py::arg("color"));
}