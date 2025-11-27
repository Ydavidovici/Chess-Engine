#include "main.h"

// ——— Constructor / Destructor ———
Engine::Engine() {
    board_.initialize();
    history_.clear();
}
Engine::~Engine() = default;

// ——— Position control ———
void Engine::reset() {
    board_.initialize();
    history_.clear();
}

bool Engine::setPosition(const std::string &fen) {
    try {
        board_.loadFEN(fen);
        history_.clear();
        return true;
    } catch (...) {
        return false;
    }
}

std::string Engine::getFEN() const {
    return board_.toFEN();
}

// ——— Evaluation ———
int Engine::evaluateCurrentPosition() const {
    Evaluator eval;
    return eval.evaluate(board_, board_.sideToMove());
}

// ——— Apply/undo moves & legal moves ———
bool Engine::applyMove(const std::string &uci) {
    Move m = Move::fromUCI(uci);
    if (!board_.makeMove(m)) return false;
    history_.push_back(uci);
    return true;
}

bool Engine::undoMove() {
    if (history_.empty()) return false;
    board_.unmakeMove();
    history_.pop_back();
    return true;
}

std::vector<std::string> Engine::legalMoves() const {
    auto mv = board_.generateLegalMoves();
    std::vector<std::string> out;
    out.reserve(mv.size());
    for (auto &m : mv) out.push_back(m.toString());
    return out;
}

std::string Engine::playMove(const PlaySettings &settings) {
    TranspositionTable tt(settings.tt_size_mb * 1024 * 1024);
    TimeManager tm;
    tm.start(settings.time_left_ms, settings.increment_ms, settings.moves_to_go);

    Evaluator eval;
    Search searcher(eval, tt);
    Move best;

    if (settings.time_left_ms > 0) {
        best = searcher.findBestMove(board_, board_.sideToMove(), settings.depth, tm);
    } else {
        best = searcher.findBestMove(board_, board_.sideToMove(), settings.depth);
    }

    board_.makeMove(best);
    std::string uci = best.toString();
    history_.push_back(uci);
    return uci;
}


// ——— Game‐over & history ———
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