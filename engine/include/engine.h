#pragma once

#include <string>
#include "board.h"
#include "move.h"
#include "evaluator.h"

/**
 * @brief The Engine class ties together the Board and Evaluator
 *        to provide a full chess engine interface.
 */
class Engine {
public:
    /**
     * @brief Construct a new Engine, initializing the board and evaluator.
     */
    Engine();

    /**
     * @brief Reset the board to the standard starting position.
     */
    void newGame();

    /**
     * @brief Load an arbitrary position from a FEN string.
     * @param fen A Forsyth–Edwards Notation string.
     */
    void loadFEN(const std::string& fen) {
        board.loadFEN(fen);
    }

    /**
     * @brief Print the current board state to stdout.
     */
    void printBoard() const;

    /**
     * @brief Attempt to make a move on the board.
     * @param m The Move to apply.
     * @param c The Color of the side making the move.
     * @return True if the move was legal and applied; false otherwise.
     */
    bool makeMove(const Move& m, Color c);

    /**
     * @brief Compute a static evaluation of the current board position.
     * @return A signed integer score: positive favors White, negative favors Black.
     */
    int evaluateBoard() const;

    /**
     * @brief Find the best move from the current position using iterative deepening.
     * @param maxDepth The maximum search depth.
     * @param side The side to move (Color::WHITE or Color::BLACK).
     * @return The Move determined to be best.
     */
    Move findBestMove(int maxDepth, Color side);

private:
    Board       board;
    Evaluator   evaluator;
};