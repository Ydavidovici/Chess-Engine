#include <iostream>
#include "board.h"
#include "engine.h"
#include "move.h"

void testBoardInitialization() {
    Board board;
    board.initializeBoard();
    board.printBoard();
    std::cout << "Board initialized and printed successfully." << std::endl;
}

void testEngineInitialization() {
    Engine engine;
    engine.initialize();
    std::cout << "Engine initialized successfully." << std::endl;
    std::cout << "Current board state:\n" << engine.getBoardState() << std::endl;
}

void testMoveParsing() {
    try {
        Move move("e2e4");
        std::cout << "Move parsed successfully: " << move.getMove() << std::endl;
        std::cout << "Start: (" << move.getStartRow() << ", " << move.getStartCol() << "), "
                  << "End: (" << move.getEndRow() << ", " << move.getEndCol() << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse move: " << e.what() << std::endl;
    }
}

void testEngineMove() {
    Engine engine;
    engine.initialize();
    Move move("e2e4");
    if (engine.makeMove(move)) {
        std::cout << "Move made successfully." << std::endl;
    } else {
        std::cerr << "Move failed." << std::endl;
    }
    std::cout << "Current board state after move:\n" << engine.getBoardState() << std::endl;
}

int main() {
    testBoardInitialization();
    testEngineInitialization();
    testMoveParsing();
    testEngineMove();
    return 0;
}
