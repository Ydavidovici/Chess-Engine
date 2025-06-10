// engine/src/engine_pybind.cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "engine.h"

namespace py = pybind11;

PYBIND11_MODULE(pyengine, m) {
    m.doc() = "Chess engine Python bindings";

    py::class_<Engine>(m, "Engine")
        .def(py::init<>())
        .def("print_board", &Engine::printBoard)
        .def("make_move", [](Engine &e, const std::string &uci, const std::string &color) {
            // UCI format: e.g. "e2e4" or "e7e8Q"
            int start = (uci[0]-'a') + (uci[1]-'1')*8;
            int end   = (uci[2]-'a') + (uci[3]-'1')*8;
            char promo = uci.size() == 5 ? uci[4] : '\0';
            Move m(start, end, promo ? MoveType::PROMOTION : MoveType::NORMAL, promo);
            Color c = (color == "white" ? Color::WHITE : Color::BLACK);
            return e.makeMove(m, c);
        })
        .def("last_move", &Engine::getLastMove);
}
