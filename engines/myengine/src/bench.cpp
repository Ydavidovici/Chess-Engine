#include "bench.h"
#include "main.h"
#include <iostream>
#include <chrono>

const std::vector<std::string> Bench::BENCH_FENS = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
};

void Bench::run(Engine& engine) {
    std::cout << "--- Starting Benchmark Suite ---\n";
    benchmarkEval(engine);
    std::cout << "\n";
    benchmarkSearch(engine, 9);
    std::cout << "--- Benchmark Complete ---\n";
}

void Bench::benchmarkEval(Engine& engine) {
    std::cout << "[Running Eval Throughput Test]\n";

    long long count = 0;
    auto start = std::chrono::high_resolution_clock::now();

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 2000)
            break;

        for (const auto& fen : BENCH_FENS) {
            engine.setPosition(fen);
            volatile int score = engine.getEvaluator().evaluate(engine.getBoard(), engine.getBoard().sideToMove());
            count++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;

    std::cout << "Total Evals: " << count << "\n";
    std::cout << "Time:        " << duration << "s\n";
    std::cout << "EPS:         " << (long long)(count / duration) << " (Evals Per Second)\n";
}

void Bench::benchmarkSearch(Engine& engine, int depth) {
    std::cout << "[Running Search Speed Test @ Depth " << depth << "]\n";

    long long totalNodes = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (const auto& fen : BENCH_FENS) {
        engine.setPosition(fen);

        PlaySettings settings{};
        settings.depth = depth;
        settings.time_left_ms = 999999;

        engine.playMove(settings);

        totalNodes += engine.getSearch().getNodes();
    }

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;

    std::cout << "Total Nodes: " << totalNodes << "\n";
    std::cout << "Time:        " << duration << "s\n";
    std::cout << "NPS:         " << (long long)(totalNodes / duration) << " (Nodes Per Second)\n";
}
