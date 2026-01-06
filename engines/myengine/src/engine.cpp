#include "main.h"

Engine::Engine() {
    board.initialize();
    history.clear();
}
Engine::~Engine() = default;

void Engine::reset() {
    board.initialize();
    history.clear();
}

bool Engine::setPosition(const std::string &fen) {
    try {
        board.loadFEN(fen);
        history.clear();
        return true;
    } catch (...) {
        return false;
    }
}

std::string Engine::getFEN() const {
    return board.toFEN();
}

int Engine::evaluateCurrentPosition() const {
    Evaluator eval;
    return eval.evaluate(board, board.sideToMove());
}

bool Engine::applyMove(const std::string &uci) {
    Move m = Move::fromUCI(uci);
    if (!board.makeMove(m)) return false;
    history.push_back(uci);
    return true;
}

bool Engine::undoMove() {
    if (history.empty()) return false;
    board.unmakeMove();
    history.pop_back();
    return true;
}

std::vector<std::string> Engine::legalMoves() const {
    auto mv = board.generateLegalMoves();
    std::vector<std::string> out;
    out.reserve(mv.size());
    for (auto &m : mv) out.push_back(m.toString());
    return out;
}

std::string Engine::playMove(const PlaySettings &settings) {
    TranspositionTable tt(1000000);
    TimeManager tm;
    // tm.start(settings.time_left_ms, settings.increment_ms, settings.moves_to_go);

    Evaluator eval;
    Search searcher(eval, tt);

    Move best = searcher.findBestMove(
        board,
        settings.depth,
        settings.time_left_ms,
        settings.increment_ms
    );

    board.makeMove(best);
    std::string uci = best.toString();
    history.push_back(uci);
    return uci;
}


bool Engine::isGameOver() const {
    return board.isCheckmate(board.sideToMove())
     || board.isStalemate(board.sideToMove())
     || board.isFiftyMoveDraw()
     || board.isThreefoldRepetition()
     || board.isInsufficientMaterial();
}

GameData Engine::getGameData() const {
    GameData gd;
    gd.moves = history;
    return gd;
}