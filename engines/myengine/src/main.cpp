#include "main.h"
#include "board.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <optional>

using CommandHandler = void(*)(const std::string& line, Engine& engine);

static void handle_uci(const std::string& line, Engine& engine);
static void handle_isready(const std::string& line, Engine& engine);
static void handle_ucinewgame(const std::string& line, Engine& engine);
static void handle_quit(const std::string& line, Engine& engine);
static void handle_position(const std::string& line, Engine& engine);
static void handle_go(const std::string& line, Engine& engine);
static void handle_bestmove(const std::string& line, Engine& engine);
static void handle_printboard(const std::string& line, Engine& engine);
static void handle_makeMove(const std::string& line, Engine& engine);

static std::unordered_map<std::string, CommandHandler> UCI_COMMANDS = {
    {"uci", handle_uci},
    {"isready", handle_isready},
    {"ucinewgame", handle_ucinewgame},
    {"quit", handle_quit},
    {"position", handle_position},
    {"go", handle_go},
    {"bestmovefromfen", handle_bestmove},
    {"printboard", handle_printboard},
    {"makemove", handle_makeMove}
};


std::optional<std::string> extractFen(const std::string& line, const std::string& expectedCmd) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd != expectedCmd) {
        return std::nullopt;
    }

    std::string fenPart;
    std::string fen;
    while (iss >> fenPart) {
        if (!fen.empty()) fen += ' ';
        fen += fenPart;
    }

    if (fen.empty()) {
        return std::nullopt;
    }

    return fen;
}

std::optional<FenAndTail> extractFenAndTail(const std::string& line, const std::string& expectedCmd) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd != expectedCmd) {
        return std::nullopt;
    }

    std::string f1, f2, f3, f4, f5, f6;
    if (!(iss >> f1 >> f2 >> f3 >> f4 >> f5 >> f6)) {
        return std::nullopt;
    }

    FenAndTail result;
    result.fen = f1 + " " + f2 + " " + f3 + " " + f4 + " " + f5 + " " + f6;

    std::string rest;
    std::getline(iss, rest);
    // trim leading spaces
    auto first = rest.find_first_not_of(" \t\r\n");
    if (first != std::string::npos)
        result.tail = rest.substr(first);
    else
        result.tail.clear();

    return result;
}


static std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

static void split_command(const std::string& line, std::string& cmd, std::string& rest) {
    std::istringstream iss(line);
    if (!(iss >> cmd)) {
        cmd.clear();
        rest.clear();
        return;
    }
    std::getline(iss, rest);
}

static bool dispatch_uci(const std::string& rawLine, Engine& engine) {
    std::string line = trim(rawLine);
    if (line.empty()) return true;

    std::string cmd;
    std::string rest;
    split_command(line, cmd, rest);

    if (cmd.empty()) return true;

    auto it = UCI_COMMANDS.find(cmd);
    if (it != UCI_COMMANDS.end()) {
        it->second(line, engine);
    }
    else {
        std::cout << "no dispatch\n";
    }
    return cmd != "quit";
}

static void handle_uci(const std::string& line, Engine& engine) {
    std::cout << "uciok\n";
    std::cout.flush();
}

static void handle_isready(const std::string& line, Engine& engine) {
    std::cout << "readyok\n";
    std::cout.flush();
}

static void handle_ucinewgame(const std::string& line, Engine& engine) {
    std::cout << "newgame\n";
    engine.reset();
    std::cout.flush();
}

static void handle_quit(const std::string& line, Engine& engine) {}

static void handle_position(const std::string& line, Engine& engine) {
    std::string cmd, rest;
    split_command(line, cmd, rest);

    std::istringstream iss(rest);
    std::string token;
    iss >> token;

    if (token == "startpos") {
        engine.reset();
    }
    else if (token == "fen") {
        std::string placement, stm, castling, ep;
        int halfmove_clock = 0;
        int fullmove_number = 1;

        iss >> placement >> stm >> castling >> ep >> halfmove_clock >> fullmove_number;

        if (!placement.empty()) {
            std::ostringstream fen;
            fen << placement << ' '
                << stm << ' '
                << castling << ' '
                << ep << ' '
                << halfmove_clock << ' '
                << fullmove_number;

            bool ok = engine.setPosition(fen.str());
            if (!ok) {
                std::cout << "info string invalid FEN in position command\n";
                std::cout.flush();
            }
        }
    }
    else {
        return;
    }

    if (iss >> token && token == "moves") {
        std::string moveUci;
        while (iss >> moveUci) {
            if (!engine.applyMove(moveUci)) {
                std::cout << "info string failed to apply move " << moveUci << "\n";
                std::cout.flush();
                break;
            }
        }
    }
}

static void handle_go(const std::string& line, Engine& engine) {
    PlaySettings settings{};
    settings.depth = 10;
    settings.tt_size_mb = 64;
    settings.time_left_ms = 0;
    settings.increment_ms = 0;
    settings.moves_to_go = 0;

    std::istringstream iss(line);
    std::string token;
    iss >> token;
    while (iss >> token) {
        if (token == "depth") {
            iss >> settings.depth;
        }
    }

    const std::string bestUci = engine.playMove(settings);

    std::cout << "bestmove " << bestUci << "\n";
    std::cout.flush();
}

static void handle_bestmove(const std::string& line, Engine& engine) {
    auto fenOpt = extractFen(line, "bestmovefromfen");
    if (!fenOpt) {return;}

    PlaySettings settings{};
    settings.depth = 10;
    settings.tt_size_mb = 64;

    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        if (token == "depth") {
            iss >> settings.depth;
        }
    }

    if (!engine.setPosition(*fenOpt)) {return;}

    const std::string bestUci = engine.playMove(settings);
    std::cout << "bestmove " << bestUci << "\n";
    std::cout.flush();
}


static void handle_printboard(const std::string& line, Engine& engine) {
    auto fenOpt = extractFen(line, "printboard");

    if (fenOpt) {
        if (!engine.setPosition(*fenOpt)) {
            std::cout << "info string invalid FEN in printboard\n";
            std::cout.flush();
            return;
        }
    }
    engine.getBoard().printBoard();

    std::cout << "printboard_done\n";
    std::cout.flush();
}


static void handle_makeMove(const std::string& line, Engine& engine) {
    auto ftOpt = extractFenAndTail(line, "makemove");
    if (!ftOpt) {
        std::cout << "info string makemove requires FEN and move\n";
        std::cout << line << "\n";
        std::cout.flush();
        return;
    }

    const auto& ft = *ftOpt;

    if (!engine.setPosition(ft.fen)) {
        std::cout << "info string makemove invalid FEN\n";
        std::cout.flush();
        return;
    }

    if (ft.tail.empty()) {
        std::cout << "info string makemove missing move\n";
        std::cout.flush();
        return;
    }

    const std::string moveStr = ft.tail;

    std::cout << "================ BEFORE ================\n";
    engine.getBoard().printBoard();

    if (!engine.applyMove(moveStr)) {
        std::cout << "info string makemove invalid move " << moveStr << "\n";
        std::cout.flush();
        return;
    }

    std::cout << "================ AFTER ================\n";
    engine.getBoard().printBoard();

    std::cout << "move_made " << moveStr << " " << engine.getFEN() << "\n";
    std::cout.flush();
}



int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Engine engine;
    engine.reset();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!dispatch_uci(line, engine)) {
            break;
        }
    }
    return 0;
}
