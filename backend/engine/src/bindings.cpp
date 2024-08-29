#include <Python.h>
#include "board.h"
#include "engine.h"
#include "evaluator.h"
#include "move.h"
#include "piece.h"
#include "player.h"

// Destructor functions for Python capsules
static void delete_board(PyObject* capsule) {
    delete static_cast<Board*>(PyCapsule_GetPointer(capsule, "Board"));
}

static void delete_engine(PyObject* capsule) {
    delete static_cast<Engine*>(PyCapsule_GetPointer(capsule, "Engine"));
}

static void delete_move(PyObject* capsule) {
    delete static_cast<Move*>(PyCapsule_GetPointer(capsule, "Move"));
}

// Board bindings
static PyObject* py_initialize_board(PyObject* self, PyObject* args) {
    Board* board = new Board();
    board->initializeBoard();
    return PyCapsule_New(board, "Board", delete_board);
}

static PyObject* py_print_board(PyObject* self, PyObject* args) {
    PyObject* board_capsule;
    if (!PyArg_ParseTuple(args, "O", &board_capsule)) {
        return NULL;
    }
    Board* board = static_cast<Board*>(PyCapsule_GetPointer(board_capsule, "Board"));
    if (!board) {
        return NULL;
    }
    board->printBoard();
    Py_RETURN_NONE;
}

static PyObject* py_get_board_state(PyObject* self, PyObject* args) {
    PyObject* board_capsule;
    if (!PyArg_ParseTuple(args, "O", &board_capsule)) {
        return NULL;
    }
    Board* board = static_cast<Board*>(PyCapsule_GetPointer(board_capsule, "Board"));
    if (!board) {
        return NULL;
    }
    std::string state = board->getBoardState();
    return Py_BuildValue("s", state.c_str());
}

// Engine bindings
static PyObject* py_initialize_engine(PyObject* self, PyObject* args) {
    Engine* engine = new Engine();
    engine->initialize();
    return PyCapsule_New(engine, "Engine", delete_engine);
}

static PyObject* py_make_move(PyObject* self, PyObject* args) {
    PyObject* engine_capsule;
    const char* move_str;
    if (!PyArg_ParseTuple(args, "Os", &engine_capsule, &move_str)) {
        return NULL;
    }
    Engine* engine = static_cast<Engine*>(PyCapsule_GetPointer(engine_capsule, "Engine"));
    if (!engine) {
        return NULL;
    }
    Move move(move_str);
    if (engine->makeMove(move)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject* py_get_board_state_engine(PyObject* self, PyObject* args) {
    PyObject* engine_capsule;
    if (!PyArg_ParseTuple(args, "O", &engine_capsule)) {
        return NULL;
    }
    Engine* engine = static_cast<Engine*>(PyCapsule_GetPointer(engine_capsule, "Engine"));
    if (!engine) {
        return NULL;
    }
    std::string state = engine->getBoardState();
    return Py_BuildValue("s", state.c_str());
}

static PyObject* py_evaluate_board(PyObject* self, PyObject* args) {
    PyObject* engine_capsule;
    if (!PyArg_ParseTuple(args, "O", &engine_capsule)) {
        return NULL;
    }
    Engine* engine = static_cast<Engine*>(PyCapsule_GetPointer(engine_capsule, "Engine"));
    if (!engine) {
        return NULL;
    }
    int evaluation = engine->evaluateBoard();
    return Py_BuildValue("i", evaluation);
}

// Move bindings
static PyObject* py_create_move(PyObject* self, PyObject* args) {
    const char* move_str;
    if (!PyArg_ParseTuple(args, "s", &move_str)) {
        return NULL;
    }
    Move* move = new Move(move_str);
    return PyCapsule_New(move, "Move", delete_move);
}

static PyObject* py_get_move(PyObject* self, PyObject* args) {
    PyObject* move_capsule;
    if (!PyArg_ParseTuple(args, "O", &move_capsule)) {
        return NULL;
    }
    Move* move = static_cast<Move*>(PyCapsule_GetPointer(move_capsule, "Move"));
    if (!move) {
        return NULL;
    }
    std::string move_str = move->getMove();
    return Py_BuildValue("s", move_str.c_str());
}

// Method definitions
static PyMethodDef ChessEngineMethods[] = {
        {"initialize_board", py_initialize_board, METH_NOARGS, "Initialize the board"},
        {"print_board", py_print_board, METH_VARARGS, "Print the board"},
        {"get_board_state", py_get_board_state, METH_VARARGS, "Get the board state"},
        {"initialize_engine", py_initialize_engine, METH_NOARGS, "Initialize the engine"},
        {"make_move", py_make_move, METH_VARARGS, "Make a move"},
        {"get_board_state_engine", py_get_board_state_engine, METH_VARARGS, "Get the board state from the engine"},
        {"evaluate_board", py_evaluate_board, METH_VARARGS, "Evaluate the board"},
        {"create_move", py_create_move, METH_VARARGS, "Create a move"},
        {"get_move", py_get_move, METH_VARARGS, "Get the move"},
        {NULL, NULL, 0, NULL}  // Sentinel
};

// Module definition
static struct PyModuleDef ChessEngineModule = {
        PyModuleDef_HEAD_INIT,
        "chess_engine",
        NULL,  // Optional module documentation string
        -1,
        ChessEngineMethods
};

// Module initialization
PyMODINIT_FUNC PyInit_chess_engine(void) {
    return PyModule_Create(&ChessEngineModule);
}
