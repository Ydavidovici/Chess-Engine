// src/main.cpp
#include "main.h"
#include <iostream>
#include <string>
#include <sstream>

// =======================
// Minimal UCI front-end
// =======================

static std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

static void handle_uci() {
    std::cout << "id name MyEngine\n";
    std::cout << "id author Yaakov\n";
    std::cout << "uciok\n";
    std::cout.flush();
}

static void handle_isready() {
    std::cout << "readyok\n";
    std::cout.flush();
}

static void handle_position_basic(const std::string& line, Engine& eng) {
    if (line.find("startpos") != std::string::npos) {
        eng.reset();
    }
}

static void handle_go_basic(const std::string& line, Engine& eng) {
    PlaySettings settings{};
    settings.depth        = 10;
    settings.tt_size_mb   = 64;
    settings.time_left_ms = 0;
    settings.increment_ms = 0;
    settings.moves_to_go  = 0;

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

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Engine eng;
    eng.reset();

    std::string line;
    while (std::getline(std::cin, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line == "uci") {
            handle_uci();
        } else if (line == "isready") {
            handle_isready();
        } else if (line.rfind("position", 0) == 0) {
            handle_position_basic(line, eng);
        } else if (line.rfind("go", 0) == 0) {
            handle_go_basic(line, eng);
        } else if (line == "ucinewgame") {
            eng.reset();
        } else if (line == "quit") {
            break;
        }
    }

    return 0;
}
