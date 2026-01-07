#pragma once
#include <string>
#include <vector>

class Engine;

class Bench {
public:
    static void run(Engine& engine);

    static void benchmarkEval(Engine& engine);
    static void benchmarkSearch(Engine& engine, int depth);

private:
    static const std::vector<std::string> BENCH_FENS;
};