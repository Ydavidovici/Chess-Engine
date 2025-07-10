/* src/bindings.cpp */
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "main.h"

namespace py = pybind11;

PYBIND11_MODULE(chessengine, m) {
m.doc() = "C++ Chess Engine Python Bindings";

py::class_<PlaySettings>(m, "PlaySettings")
.def(py::init<>())
.def_readwrite("depth", &PlaySettings::depth)
.def_readwrite("tt_size_mb", &PlaySettings::tt_size_mb)
.def_readwrite("time_left_ms", &PlaySettings::time_left_ms)
.def_readwrite("increment_ms", &PlaySettings::increment_ms)
.def_readwrite("moves_to_go", &PlaySettings::moves_to_go);

py::class_<GameData>(m, "GameData")
.def_readonly("moves", &GameData::moves);

py::class_<Engine>(m, "Engine")
.def(py::init<>())
.def("apply_move", &Engine::applyMove, py::arg("uci"))
.def("play_move", &Engine::playMove, py::arg("settings"))
.def("is_game_over", &Engine::isGameOver)
.def("get_game_data", &Engine::getGameData);
}
