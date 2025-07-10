/* src/main.cpp */
#include "main.h"

Engine::Engine() {
    board_.initialize();
    history_.clear();
}

bool Engine::applyMove(const std::string &uci) {
    Move m = Move::fromUCI(uci);
    if (!board_.makeMove(m)) return false;
    history_.push_back(uci);
    return true;
}

std::string Engine::playMove(const PlaySettings &settings) {
    // wire up search objects
    TranspositionTable tt(settings.tt_size_mb * 1024 * 1024);
    TimeManager tm;
    tm.start(settings.time_left_ms, settings.increment_ms, settings.moves_to_go);
    Evaluator eval;
    Search searcher(eval, tt);
    Move best = (settings.time_left_ms > 0)
                ? searcher.findBestMove(board_, board_.sideToMove(), settings.depth, tm)
                : searcher.findBestMove(board_, board_.sideToMove(), settings.depth);
    board_.makeMove(best);
    std::string uci = best.toString();
    history_.push_back(uci);
    return uci;
}

bool Engine::isGameOver() const {
    return board_.isCheckmate(board_.sideToMove())
           || board_.isStalemate(board_.sideToMove())
           || board_.isFiftyMoveDraw()
           || board_.isThreefoldRepetition()
           || board_.isInsufficientMaterial();
}

GameData Engine::getGameData() const {
    GameData gd;
    gd.moves = history_;
    return gd;
}