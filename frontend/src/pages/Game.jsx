import React, { useState, useEffect } from "react";
import { Chess } from "chess.js";
import { Chessboard } from "react-chessboard";
import { go, resetGame } from "../services/api.js";
import BenchmarkControl from "./BenchmarkControl";
import { Play, Activity, RotateCcw, Cpu, Trophy, AlertTriangle } from "lucide-react";

export default function Game() {
    const [activeTab, setActiveTab] = useState("play");

    const [game, setGame] = useState(() => new Chess());
    const [gameStatus, setGameStatus] = useState(null); // "Checkmate", "Draw", etc.

    const [engineThinking, setEngineThinking] = useState(false);
    const [lastMove, setLastMove] = useState(null);
    const [engineDepth, setEngineDepth] = useState(10);

    const checkGameOver = (gameInstance) => {
        if (gameInstance.isCheckmate()) {
            const winner = gameInstance.turn() === "w" ? "Black" : "White";
            setGameStatus(`Checkmate! ${winner} wins.`);
            return true;
        }
        if (gameInstance.isDraw()) {
            setGameStatus("Draw!");
            return true;
        }
        if (gameInstance.isGameOver()) {
            setGameStatus("Game Over!");
            return true;
        }
        return false;
    };

    const safeGameMutate = (modify) => {
        setGame((g) => {
            const update = new Chess(g.fen());
            modify(update);
            return update;
        });
    };

    const makeEngineMove = async (currentFen) => {
        if (checkGameOver(new Chess(currentFen))) return;

        setEngineThinking(true);
        try {
            const response = await go({
                fen: currentFen,
                options: { depth: engineDepth }
            });

            const bestMove = response.bestMove;

            if (!bestMove || bestMove.length < 4 || bestMove.includes("`")) {
                console.warn("Engine returned invalid move (Resignation?):", bestMove);
                return;
            }

            safeGameMutate((g) => {
                try {
                    const moveResult = g.move({
                        from: bestMove.substring(0, 2),
                        to: bestMove.substring(2, 4),
                        promotion: bestMove.length > 4 ? bestMove[4] : 'q',
                    });

                    if (moveResult) {
                        setLastMove(bestMove);
                        checkGameOver(g);
                    }
                } catch (e) {
                    console.error("Engine tried illegal move:", bestMove);
                }
            });
        } catch (error) {
            console.error("Engine API failed:", error);
        } finally {
            setEngineThinking(false);
        }
    };

    const onDrop = (sourceSquare, targetSquare) => {
        if (engineThinking || gameStatus) return false;

        const gameCopy = new Chess(game.fen());
        let move = null;

        try {
            move = gameCopy.move({
                from: sourceSquare,
                to: targetSquare,
                promotion: "q",
            });
        } catch (e) {
            return false;
        }

        setGame(gameCopy);

        if (checkGameOver(gameCopy)) {
            return true;
        }

        const newFen = gameCopy.fen();
        setTimeout(() => {
            makeEngineMove(newFen);
        }, 200);

        return true;
    };

    const handleReset = async () => {
        setGame(new Chess());
        setLastMove(null);
        setGameStatus(null);
        setEngineThinking(false);
        await resetGame();
    };

    return (
        <div className="min-h-screen bg-gray-900 text-gray-100 font-sans">
            <header className="bg-gray-800 border-b border-gray-700 p-4 shadow-md">
                <div className="max-w-6xl mx-auto flex justify-between items-center">
                    <h1 className="text-xl font-bold bg-gradient-to-r from-blue-400 to-purple-500 bg-clip-text text-transparent">
                        ♞ MyEngine v1.0
                    </h1>
                    <div className="flex gap-2 bg-gray-900 p-1 rounded-lg border border-gray-700">
                        <button
                            onClick={() => setActiveTab("play")}
                            className={`px-4 py-2 rounded-md flex items-center gap-2 text-sm font-medium transition-all ${
                                activeTab === "play" ? "bg-blue-600 text-white" : "text-gray-400 hover:text-white"
                            }`}
                        >
                            <Play size={16} /> Play
                        </button>
                        <button
                            onClick={() => setActiveTab("bench")}
                            className={`px-4 py-2 rounded-md flex items-center gap-2 text-sm font-medium transition-all ${
                                activeTab === "bench" ? "bg-purple-600 text-white" : "text-gray-400 hover:text-white"
                            }`}
                        >
                            <Activity size={16} /> Benchmark
                        </button>
                    </div>
                </div>
            </header>

            <main className="max-w-6xl mx-auto p-6">
                {activeTab === "play" ? (
                    <div className="grid grid-cols-1 lg:grid-cols-3 gap-8">

                        <div className="lg:col-span-2 relative flex justify-center bg-gray-800/50 p-6 rounded-xl border border-gray-700 shadow-2xl backdrop-blur-sm">
                            <div className="w-full max-w-[600px] aspect-square">
                                <Chessboard
                                    position={game.fen()}
                                    onPieceDrop={onDrop}
                                    boardOrientation="white"
                                    customDarkSquareStyle={{ backgroundColor: "#374151" }}
                                    customLightSquareStyle={{ backgroundColor: "#9CA3AF" }}
                                    animationDuration={200}
                                />
                            </div>

                            {gameStatus && (
                                <div className="absolute inset-0 bg-black/60 flex items-center justify-center rounded-xl z-10 backdrop-blur-sm">
                                    <div className="bg-gray-800 p-8 rounded-2xl border border-gray-600 shadow-2xl text-center transform scale-110">
                                        <Trophy className="w-16 h-16 text-yellow-400 mx-auto mb-4" />
                                        <h2 className="text-3xl font-bold text-white mb-2">{gameStatus}</h2>
                                        <button
                                            onClick={handleReset}
                                            className="mt-6 px-6 py-2 bg-blue-600 hover:bg-blue-500 text-white rounded-lg font-bold transition-all"
                                        >
                                            New Game
                                        </button>
                                    </div>
                                </div>
                            )}
                        </div>

                        <div className="space-y-6">
                            <div className="bg-gray-800 p-6 rounded-xl border border-gray-700 shadow-lg">
                                <h2 className="text-gray-400 text-xs font-bold uppercase tracking-wider mb-4 flex items-center gap-2">
                                    <Cpu size={14} /> Engine Status
                                </h2>

                                <div className="flex items-center justify-between mb-6">
                                    <div className="flex items-center gap-3">
                                        <div className={`w-3 h-3 rounded-full ${engineThinking ? "bg-yellow-400 animate-pulse" : "bg-green-500"}`} />
                                        <span className="font-mono text-sm">
                                            {engineThinking ? "Thinking..." : "Waiting for move"}
                                        </span>
                                    </div>
                                    {lastMove && (
                                        <span className="bg-gray-700 px-3 py-1 rounded text-xs font-mono text-blue-300">
                                            Last: {lastMove}
                                        </span>
                                    )}
                                </div>

                                <div className="space-y-4">
                                    <div>
                                        <label className="text-xs text-gray-500 mb-1 block">Search Depth (Difficulty)</label>
                                        <input
                                            type="range"
                                            min="1" max="20"
                                            value={engineDepth}
                                            onChange={(e) => setEngineDepth(parseInt(e.target.value))}
                                            className="w-full h-2 bg-gray-700 rounded-lg appearance-none cursor-pointer accent-blue-500"
                                        />
                                        <div className="flex justify-between text-xs text-gray-400 mt-1">
                                            <span>Fast (1)</span>
                                            <span className="text-white font-bold">{engineDepth} Ply</span>
                                            <span>Strong (20)</span>
                                        </div>
                                    </div>
                                </div>

                                <button
                                    onClick={handleReset}
                                    className="w-full mt-6 py-3 bg-gray-700 hover:bg-gray-600 border border-gray-600 rounded-lg flex items-center justify-center gap-2 transition-all text-sm font-bold"
                                >
                                    <RotateCcw size={16} /> Reset Game
                                </button>
                            </div>

                            <div className="bg-gray-800 p-6 rounded-xl border border-gray-700 h-64 overflow-y-auto">
                                <h3 className="text-gray-400 text-xs font-bold uppercase tracking-wider mb-2">Move History</h3>
                                <div className="text-sm font-mono text-gray-500 leading-relaxed">
                                    {game.history().map((move, i) => (
                                        <span key={i} className={i % 2 === 0 ? "text-gray-300" : "mr-2"}>
                                            {i % 2 === 0 ? `${Math.floor(i/2) + 1}. ` : ""}{move}
                                        </span>
                                    ))}
                                    {game.history().length === 0 && <span className="opacity-30">No moves yet.</span>}
                                </div>
                            </div>
                        </div>
                    </div>
                ) : (
                    <BenchmarkControl />
                )}
            </main>
        </div>
    );
}