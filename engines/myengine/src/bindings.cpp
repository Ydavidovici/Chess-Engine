#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "types.h"
#include "move.h"
#include "board.h"
#include "main.h"

namespace py = pybind11;

PYBIND11_MODULE(chessengine, m) {
    m.doc() = "C++ Chess Engine Python Bindings";

    py::enum_<Color>(m, "Color")
    .value("WHITE", Color::WHITE)
    .value("BLACK", Color::BLACK)
    .export_values();


    py::class_<Move>(m, "Move")
        .def_static("fromUCI", &Move::fromUCI, py::arg("uci"))
        .def("toString",       &Move::toString)
        .def("__str__",        &Move::toString)
        ;


    py::class_<PlaySettings>(m, "PlaySettings")
        .def(py::init<>())
        .def_readwrite("depth",        &PlaySettings::depth)
        .def_readwrite("tt_size_mb",   &PlaySettings::tt_size_mb)
        .def_readwrite("time_left_ms", &PlaySettings::time_left_ms)
        .def_readwrite("increment_ms", &PlaySettings::increment_ms)
        .def_readwrite("moves_to_go",  &PlaySettings::moves_to_go)
        ;


    py::class_<GameData>(m, "GameData")
        .def_readonly("moves", &GameData::moves)
        ;

    py::enum_<Board::PieceIndex>(m, "PieceIndex")
        .value("PAWN",   Board::PAWN)
        .value("KNIGHT", Board::KNIGHT)
        .value("BISHOP", Board::BISHOP)
        .value("ROOK",   Board::ROOK)
        .value("QUEEN",  Board::QUEEN)
        .value("KING",   Board::KING)
        .export_values()
        ;

    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def("initialize",           &Board::initialize)
        .def("load_fen",             &Board::loadFEN,       py::arg("fen"))
        .def("to_fen",               &Board::toFEN)
        .def("generate_pseudo_moves", &Board::generatePseudoMoves)
        .def("generate_legal_moves",  &Board::generateLegalMoves)
        .def("make_move", [](Board &b, const std::string &u){ return b.makeMove(Move::fromUCI(u)); }, py::arg("uci"))
        .def("unmake_move",          &Board::unmakeMove)
        .def("in_check",             &Board::inCheck,               py::arg("color"))
        .def("has_legal_moves",      &Board::hasLegalMoves,         py::arg("color"))
        .def("is_checkmate",         &Board::isCheckmate,           py::arg("color"))
        .def("is_stalemate",         &Board::isStalemate,           py::arg("color"))
        .def("is_fifty_move_draw",   &Board::isFiftyMoveDraw)
        .def("is_threefold_repetition", &Board::isThreefoldRepetition)
        .def("is_insufficient_material", &Board::isInsufficientMaterial)
        .def("occupancy",            &Board::occupancy,             py::arg("color"))
        .def("piece_bb",             &Board::pieceBB,               py::arg("color"), py::arg("piece_index"))
        .def("side_to_move",         &Board::sideToMove)
        .def("is_square_attacked",   &Board::is_square_attacked,    py::arg("square"), py::arg("color"))
        .def("zobrist_key",          &Board::zobristKey)
        ;

    py::class_<Engine>(m, "Engine")
        .def(py::init<>())
        .def("reset",         &Engine::reset)
        .def("set_position",  &Engine::setPosition, py::arg("fen"))
        .def("get_fen",       &Engine::getFEN)
        .def("apply_move",    &Engine::applyMove,   py::arg("uci"))
        .def("undo_move",     &Engine::undoMove)
        .def("legal_moves",   &Engine::legalMoves)
        .def("evaluate",      &Engine::evaluateCurrentPosition)
        .def("play_move",     &Engine::playMove,    py::arg("settings"))
        .def("is_game_over",  &Engine::isGameOver)
        .def("get_game_data", &Engine::getGameData)
        ;
}
