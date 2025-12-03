#include "main.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>

using CommandHandler = void(*)(const std::string& line, Engine& eng);

static void handle_uci(const std::string& line, Engine& eng);
static void handle_isready(const std::string& line, Engine& eng);
static void handle_position_basic(const std::string& line, Engine& eng);
static void handle_go_basic(const std::string& line, Engine& eng);
static void handle_ucinewgame(const std::string& line, Engine& eng);
static void handle_quit(const std::string& line, Engine& eng);
static void bestMoveFromFen(const std::string& line, Engine& eng);

static std::unordered_map<std::string, CommandHandler> UCI_COMMANDS = {
    {"uci", handle_uci},
    {"isready", handle_isready},
    {"position", handle_position_basic},
    {"go", handle_go_basic},
    {"ucinewgame", handle_ucinewgame},
    {"quit", handle_quit},
    {"bestmovefromfen", bestMoveFromFen}
};

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

static bool dispatch_uci(const std::string& rawLine, Engine& eng) {
    std::string line = trim(rawLine);
    if (line.empty()) return true;

    std::string cmd;
    std::string rest;
    split_command(line, cmd, rest);

    if (cmd.empty()) return true;

    auto it = UCI_COMMANDS.find(cmd);
    if (it != UCI_COMMANDS.end()) {
        it->second(line, eng);
    } else {
        std::cout << "no dispatch\n";
    }
    return cmd != "quit";
}

static void handle_uci(const std::string& line, Engine& eng) {
    std::cout << "uciok\n";
    std::cout.flush();
}


static void handle_isready(const std::string& line, Engine& eng) {
    std::cout << "readyok\n";
    std::cout.flush();
}

static void handle_ucinewgame(const std::string& line, Engine& eng) {
    std::cout << "newgame\n";
    std::cout.flush();
}

static void handle_quit(const std::string& line, Engine&eng) {
    std::cout << "six\n";
    std::cout << "seven\n";
}

static void handle_position_basic(const std::string& line, Engine& eng) {
    if (line.find("startpos") != std::string::npos) {
        eng.reset();
    }
}

static void handle_go_basic(const std::string& line, Engine& eng) {
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

    const std::string bestUci = eng.playMove(settings);

    std::cout << "bestmove " << bestUci << "\n";
    std::cout.flush();
}

static void bestMoveFromFen(const std::string& line, Engine& eng) {
    std::cout << "bestmove six seven\n";
    std::cout.flush();
}


// static void bestMoveFromFen(const std::string& line, Engine& eng) {
//     // line looks like: "bestmovefromfen <fen tokens...>"
//
//     std::istringstream iss(line);
//     std::string cmd;
//     iss >> cmd; // "bestmovefromfen"
//
//     // Collect the rest of the line as a FEN string
//     std::string fenPart;
//     std::string fen;
//     while (iss >> fenPart) {
//         if (!fen.empty()) fen += ' ';
//         fen += fenPart;
//     }
//
//     if (!fen.empty()) {
//         eng.setPosition(fen);  // assumes Engine::setPosition(fen) exists
//     }
//
//     PlaySettings settings{};
//     settings.depth        = 10;  // or whatever you like
//     settings.tt_size_mb   = 64;
//     settings.time_left_ms = 0;
//     settings.increment_ms = 0;
//     settings.moves_to_go  = 0;
//
//     const std::string bestUci = eng.playMove(settings);
//
//     std::cout << "bestmove " << bestUci << "\n";
//     std::cout.flush();
// }


int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Engine eng;
    eng.reset();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!dispatch_uci(line, eng)) {
            break;
        }
    }
    return 0;
}

