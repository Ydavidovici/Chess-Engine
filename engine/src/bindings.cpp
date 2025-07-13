#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "main.h"
#include "board.h"

namespace py = pybind11;

PYBIND11_MODULE(chessengine, m) {
m.doc() = "C++ Chess Engine Python Bindings";

// PlaySettings
py::class_<PlaySettings>(m, "PlaySettings")
.def(py::init<>())
.def_readwrite("depth",       &PlaySettings::depth)
.def_readwrite("tt_size_mb",  &PlaySettings::tt_size_mb)
.def_readwrite("time_left_ms",&PlaySettings::time_left_ms)
.def_readwrite("increment_ms",&PlaySettings::increment_ms)
.def_readwrite("moves_to_go", &PlaySettings::moves_to_go);

// GameData (move history)
py::class_<GameData>(m, "GameData")
.def_readonly("moves", &GameData::moves);

// Board enums & class
py::enum_<Board::PieceIndex>(m, "PieceIndex")
.value("PAWN",   Board::PAWN)
.value("KNIGHT", Board::KNIGHT)
.value("BISHOP", Board::BISHOP)
.value("ROOK",   Board::ROOK)
.value("QUEEN",  Board::QUEEN)
.value("KING",   Board::KING)
.export_values();

py::class_<Board>(m, "Board")
.def(py::init<>())
.def("initialize",            &Board::initialize)
.def("load_fen",              &Board::loadFEN,       py::arg("fen"))
.def("to_fen",                &Board::toFEN)
.def("generate_pseudo_moves", &Board::generatePseudoMoves)
.def("generate_legal_moves",  &Board::generateLegalMoves)
.def("make_move",             &Board::makeMove,      py::arg("move"))
.def("unmake_move",           &Board::unmakeMove)
.def("in_check",              &Board::inCheck,       py::arg("color"))
.def("has_legal_moves",       &Board::hasLegalMoves, py::arg("color"))
.def("is_checkmate",          &Board::isCheckmate,   py::arg("color"))
.def("is_stalemate",          &Board::isStalemate,   py::arg("color"))
.def("is_fifty_move_draw",    &Board::isFiftyMoveDraw)
.def("is_threefold_repetition", &Board::isThreefoldRepetition)
.def("is_insufficient_material", &Board::isInsufficientMaterial)
.def("occupancy",             &Board::occupancy,     py::arg("color"))
.def("piece_bb",              &Board::pieceBB,       py::arg("color"), py::arg("piece_index"))
.def("side_to_move",          &Board::sideToMove)
.def("zobrist_key",           &Board::zobristKey);

// Engine
py::class_<Engine>(m, "Engine")
.def(py::init<>())
.def("reset",             &Engine::reset)
.def("set_position",      &Engine::setPosition,  py::arg("fen"))
.def("get_fen",           &Engine::getFEN)
.def("apply_move",        &Engine::applyMove,    py::arg("uci"))
.def("undo_move",         &Engine::undoMove)
.def("legal_moves",       &Engine::legalMoves)
.def("evaluate",          &Engine::evaluateCurrentPosition)
.def("play_move",         &Engine::playMove,     py::arg("settings"))
.def("is_game_over",      &Engine::isGameOver)
.def("get_game_data",     &Engine::getGameData);
}