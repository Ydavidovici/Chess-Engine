import React, { useState } from "react";
import { runBenchmark } from "../services/api.js";

const ResultCard = ({ label, value, sub, color = "text-white" }) => (
    <div className="bg-gray-700 p-4 rounded text-center shadow-lg border border-gray-600">
        <div className="text-gray-400 text-xs uppercase tracking-wider mb-1">{label}</div>
        <div className={`text-3xl font-bold ${color}`}>{value}</div>
        {sub && <div className="text-xs text-gray-500 mt-1">{sub}</div>}
    </div>
);

const BenchmarkControl = () => {
    const [mode, setMode] = useState("time");
    const [paramValue, setParamValue] = useState(5);
    const [evalTime, setEvalTime] = useState(1);

    const [loading, setLoading] = useState(false);
    const [results, setResults] = useState(null);
    const [error, setError] = useState(null);

    const calculateTotalTime = () => {
        const POSITION_COUNT = 3;
        let searchDuration = 0;

        if (mode === "time") {
            searchDuration = paramValue * POSITION_COUNT;
        } else {
            searchDuration = (paramValue > 8 ? 10 : 2) * POSITION_COUNT;
        }

        return searchDuration + evalTime;
    };

    const totalDuration = calculateTotalTime();

    const handleRun = async () => {
        setLoading(true);
        setError(null);
        setResults(null);

        const payload = {
            mode: mode,
            evalTime: evalTime * 1000,
            ...(mode === "time" ? { timeLimit: paramValue * 1000 } : { depth: paramValue })
        };

        try {
            const response = await runBenchmark(payload);
            setResults(response.data);
        } catch (err) {
            setError(err.message || "Benchmark failed");
        } finally {
            setLoading(false);
        }
    };

    return (
        <div className="p-6 bg-gray-800 rounded-xl border border-gray-700 max-w-2xl mx-auto text-white mt-8 shadow-2xl">
            <h2 className="text-2xl font-bold mb-6 flex items-center gap-3 border-b border-gray-700 pb-4">
                <span>⚙️</span> Custom Benchmark Suite
            </h2>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-6 mb-8">
                <div className="space-y-2">
                    <label className="text-sm font-semibold text-gray-300">Search Target</label>
                    <div className="flex bg-gray-900 rounded p-1 border border-gray-600">
                        <button
                            onClick={() => setMode("time")}
                            className={`flex-1 py-2 rounded text-sm font-medium transition-all ${
                                mode === "time" ? "bg-blue-600 text-white shadow" : "text-gray-400 hover:text-white"
                            }`}
                        >
                            Fixed Time
                        </button>
                        <button
                            onClick={() => setMode("depth")}
                            className={`flex-1 py-2 rounded text-sm font-medium transition-all ${
                                mode === "depth" ? "bg-purple-600 text-white shadow" : "text-gray-400 hover:text-white"
                            }`}
                        >
                            Fixed Depth
                        </button>
                    </div>
                </div>

                <div className="space-y-2">
                    <label className="text-sm font-semibold text-gray-300">
                        {mode === "time" ? "Search Time per Position (Seconds)" : "Search Target Depth (Ply)"}
                    </label>
                    <input
                        type="number"
                        min="1"
                        max={mode === "depth" ? 20 : 60}
                        value={paramValue}
                        onChange={(e) => setParamValue(parseInt(e.target.value) || 1)}
                        className="w-full bg-gray-900 border border-gray-600 text-white p-2 rounded focus:ring-2 focus:ring-blue-500 outline-none transition-all"
                    />
                    <p className="text-[10px] text-gray-500">
                        {mode === "time"
                            ? "Engine will search as deep as possible within this time limit."
                            : "Engine will search until it reaches this exact depth."}
                    </p>
                </div>

                <div className="space-y-2 md:col-span-2 border-t border-gray-700 pt-4 mt-2">
                    <div className="flex justify-between items-center">
                        <label className="text-sm font-semibold text-gray-300">Static Eval Test Duration</label>
                        <span className="bg-gray-700 px-3 py-1 rounded text-sm w-16 text-center font-mono">
                            {evalTime}s
                        </span>
                    </div>
                    <div className="flex items-center gap-4">
                        <input
                            type="range"
                            min="0"
                            max="5"
                            step="0.5"
                            value={evalTime}
                            onChange={(e) => setEvalTime(parseFloat(e.target.value))}
                            className="flex-1 h-2 bg-gray-600 rounded-lg appearance-none cursor-pointer accent-green-500"
                        />
                    </div>
                    <p className="text-[10px] text-gray-500">
                        Measures raw EPS (Evals Per Second). Does not perform any search.
                    </p>
                </div>
            </div>

            <div className="bg-gray-900/50 rounded p-3 mb-4 flex justify-between items-center border border-gray-700">
                <span className="text-sm text-gray-400">Estimated Total Runtime:</span>
                <span className="font-mono text-lg font-bold text-yellow-400">
                    ~{totalDuration}s
                </span>
            </div>

            <button
                onClick={handleRun}
                disabled={loading}
                className={`w-full py-4 rounded-lg font-bold text-lg transition-all shadow-lg
                    ${loading
                    ? "bg-gray-700 cursor-wait text-gray-400"
                    : "bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-500 hover:to-purple-500 text-white transform hover:scale-[1.02]"
                }`}
            >
                {loading ? (
                    <div className="flex items-center justify-center gap-2">
                        <div className="w-5 h-5 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                        Running Suite...
                    </div>
                ) : (
                    "Run Benchmark"
                )}
            </button>

            {error && (
                <div className="mt-6 p-4 bg-red-900/30 border border-red-500/50 rounded-lg text-red-200 text-center animate-fade-in">
                    ⚠️ {error}
                </div>
            )}

            {results && !loading && (
                <div className="mt-8 space-y-6 animate-slide-up">
                    <div className="h-px bg-gray-700" />

                    <div className="grid grid-cols-2 gap-4">
                        <ResultCard
                            label="NPS (Speed)"
                            value={results.nps.toLocaleString()}
                            sub="Nodes / Second"
                            color="text-blue-400"
                        />
                        <ResultCard
                            label="EPS (Throughput)"
                            value={results.eps.toLocaleString()}
                            sub="Evals / Second"
                            color="text-green-400"
                        />
                    </div>

                    <div className="bg-gray-900/50 p-6 rounded-lg border border-gray-700">
                        <h3 className="text-sm font-bold text-gray-400 uppercase tracking-wider mb-6 text-center">
                            Search Efficiency Breakdown
                        </h3>
                        <div className="grid grid-cols-3 gap-6 text-center">
                            <div className="flex flex-col items-center">
                                <div className="text-3xl font-bold text-yellow-400 mb-1">{results.ordering}%</div>
                                <div className="text-xs text-gray-400">Move Ordering</div>
                                <div className="text-[10px] text-gray-600 mt-1">Goal: &gt;85%</div>
                            </div>
                            <div className="flex flex-col items-center border-l border-gray-700">
                                <div className="text-3xl font-bold text-purple-400 mb-1">{results.qSearch}%</div>
                                <div className="text-xs text-gray-400">Q-Search Load</div>
                                <div className="text-[10px] text-gray-600 mt-1">Goal: &lt;50%</div>
                            </div>
                            <div className="flex flex-col items-center border-l border-gray-700">
                                <div className="text-3xl font-bold text-pink-400 mb-1">{results.ttHit}%</div>
                                <div className="text-xs text-gray-400">TT Hit Rate</div>
                                <div className="text-[10px] text-gray-600 mt-1">Higher is better</div>
                            </div>
                        </div>
                    </div>

                    <div className="text-center">
                         <span className="inline-block bg-gray-700/50 px-3 py-1 rounded text-xs text-gray-400">
                            Processed {results.nodes.toLocaleString()} nodes in {results.time}s
                         </span>
                    </div>
                </div>
            )}
        </div>
    );
};

export default BenchmarkControl;