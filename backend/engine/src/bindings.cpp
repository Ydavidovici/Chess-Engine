#include <Python.h>
#include "board.h"
#include "engine.h"
#include "evaluator.h"
#include "move.h"
#include "piece.h"
#include "player.h"

// Board bindings
static PyObject* py_initialize_board(PyObject* self, PyObject* args) {
    Board* board = new Board();
    board->initializeBoard();
    return PyCapsule_New(board, "Board", NULL);
}

static PyObject* py_print_board(PyObject* self, PyObject* args) {
    PyObject* board_capsule;
    if (!PyArg_ParseTuple(args, "O", &board_capsule)) {
        return NULL;
    }
    Board* board = (Board*)PyCapsule_GetPointer(board_capsule, "Board");
    board->printBoard();
    Py_RETURN_NONE;
}

static PyObject* py_get_board_state(PyObject* self, PyObject* args) {
    PyObject* board_capsule;
    if (!PyArg_ParseTuple(args, "O", &board_capsule)) {
        return NULL;
    }
    Board* board = (Board*)PyCapsule_GetPointer(board_capsule, "Board");
    std::string state = board->getBoardState();
    return Py_BuildValue("s", state.c_str());
}

// Move bindings
static PyObject* py_create_move(PyObject* self, PyObject* args) {
    const char* move_str;
    if (!PyArg_ParseTuple(args, "s", &move_str)) {
        return NULL;
    }
    Move* move = new Move(move_str);
    return PyCapsule_New(move, "Move", NULL);
}

static PyObject* py_get_move(PyObject* self, PyObject* args) {
    PyObject* move_capsule;
    if (!PyArg_ParseTuple(args, "O", &move_capsule)) {
        return NULL;
    }
    Move* move = (Move*)PyCapsule_GetPointer(move_capsule, "Move");
    std::string move_str = move->getMove();
    return Py_BuildValue("s", move_str.c_str());
}

// Engine bindings
static PyObject* py_initialize_engine(PyObject* self, PyObject* args) {
    Engine* engine = new Engine();
    engine->initialize();
    return PyCapsule_New(engine, "Engine", NULL);
}

static PyObject* py_make_move(PyObject* self, PyObject* args) {
    PyObject* engine_capsule;
    PyObject* move_capsule;
    if (!PyArg_ParseTuple(args, "OO", &engine_capsule, &move_capsule)) {
        return NULL;
    }
    Engine* engine = (Engine*)PyCapsule_GetPointer(engine_capsule, "Engine");
    Move* move = (Move*)PyCapsule_GetPointer(move_capsule, "Move");
    engine->makeMove(move->getMove());  // Pass the move string
    Py_RETURN_NONE;
}

static PyObject* py_get_board_state_engine(PyObject* self, PyObject* args) {
    PyObject* engine_capsule;
    if (!PyArg_ParseTuple(args, "O", &engine_capsule)) {
        return NULL;
    }
    Engine* engine = (Engine*)PyCapsule_GetPointer(engine_capsule, "Engine");
    std::string state = engine->getBoardState();
    return Py_BuildValue("s", state.c_str());
}

static PyObject* py_evaluate_board(PyObject* self, PyObject* args) {
    PyObject* engine_capsule;
    if (!PyArg_ParseTuple(args, "O", &engine_capsule)) {
        return NULL;
    }
    Engine* engine = (Engine*)PyCapsule_GetPointer(engine_capsule, "Engine");
    int evaluation = engine->evaluateBoard();
    return Py_BuildValue("i", evaluation);
}

// Method definitions
static PyMethodDef ChessEngineMethods[] = {
        {"initialize_board", py_initialize_board, METH_NOARGS, "Initialize the board"},
        {"print_board", py_print_board, METH_VARARGS, "Print the board"},
        {"get_board_state", py_get_board_state, METH_VARARGS, "Get the board state"},
        {"create_move", py_create_move, METH_VARARGS, "Create a move"},
        {"get_move", py_get_move, METH_VARARGS, "Get the move"},
        {"initialize_engine", py_initialize_engine, METH_NOARGS, "Initialize the engine"},
        {"make_move", py_make_move, METH_VARARGS, "Make a move"},
        {"get_board_state_engine", py_get_board_state_engine, METH_VARARGS, "Get the board state from the engine"},
        {"evaluate_board", py_evaluate_board, METH_VARARGS, "Evaluate the board"},
        {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef ChessEngineModule = {
        PyModuleDef_HEAD_INIT,
        "chess_engine",
        NULL,
        -1,
        ChessEngineMethods
};

// Module initialization
PyMODINIT_FUNC PyInit_chess_engine(void) {
    return PyModule_Create(&ChessEngineModule);
}
