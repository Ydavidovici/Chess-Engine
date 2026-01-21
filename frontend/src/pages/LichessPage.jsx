import React, { useState, useEffect } from "react";
import {
    startLichessBot,
    stopLichessBot,
    getLichessStatus,
    createChallenge,
    createOpenChallenge,
    createAiChallenge,
    challengeWeakestBot
} from "../services/api.js";

export default function LichessPage() {
    const [token, setToken] = useState("");
    const [isRunning, setIsRunning] = useState(false);
    const [botProfile, setBotProfile] = useState(null);
    const [activeGames, setActiveGames] = useState([]);
    const [statusMessage, setStatusMessage] = useState("");
    const [error, setError] = useState(null);

    const [opponent, setOpponent] = useState("maia1");
    const [stockfishLevel, setStockfishLevel] = useState(1);

    useEffect(() => {
        const fetchStatus = async () => {
            try {
                const data = await getLichessStatus();
                setIsRunning(data.running);
                setBotProfile(data.profile);
                setActiveGames(data.activeGames || []);
            } catch (err) {
                console.error("Failed to poll status", err);
            }
        };
        fetchStatus();
        const interval = setInterval(fetchStatus, 2000);
        return () => clearInterval(interval);
    }, []);

    const handleStart = async () => {
        setError(null);
        setStatusMessage("Starting...");
        try {
            const res = await startLichessBot(token || undefined);
            setStatusMessage(res.message);
            setIsRunning(true);
        } catch (err) {
            setError(err.response?.data?.error || err.message);
            setStatusMessage("");
        }
    };

    const handleStop = async () => {
        setError(null);
        setStatusMessage("Stopping...");
        try {
            const res = await stopLichessBot();
            setStatusMessage(res.message);
            setIsRunning(false);
            setBotProfile(null);
            setActiveGames([]);
        } catch (err) {
            setError(err.message);
        }
    };

    const handleBotChallenge = async () => {
        setError(null);
        setStatusMessage(`Challenging ${opponent}...`);
        try {
            await createChallenge(opponent, 180, 0);
            setStatusMessage(`Challenge sent to ${opponent}!`);
        } catch (err) {
            setError(err.response?.data?.error || "Challenge failed");
        }
    };

    const handleOpenChallenge = async () => {
        setError(null);
        setStatusMessage("Creating Open Challenge...");
        try {
            await createOpenChallenge(180, 0);
            setStatusMessage("Open Challenge Created! Waiting for opponent...");
        } catch (err) {
            setError(err.response?.data?.error || "Open Challenge failed");
        }
    };

    const handleAiChallenge = async () => {
        setError(null);
        setStatusMessage(`Starting game vs Stockfish Level ${stockfishLevel}...`);
        try {
            await createAiChallenge(stockfishLevel, 180, 0);
            setStatusMessage("Game started against AI!");
        } catch (err) {
            setError(err.response?.data?.error || "AI Challenge failed");
        }
    };

    const handleWeakestChallenge = async () => {
        setError(null);
        setStatusMessage("Scanning for weakest bot online...");
        try {
            const res = await challengeWeakestBot(180, 0);
            setStatusMessage(res.message);
        } catch (err) {
            setError(err.response?.data?.error || "Scan failed");
        }
    };

    return (
        <div className="lichess-container" style={{ padding: "2rem", maxWidth: "800px", margin: "0 auto" }}>
            <h1 style={{ fontSize: "2rem", marginBottom: "1rem" }}>♞ Lichess Bot Dashboard</h1>

            <div style={{
                background: "#2a2a2a", padding: "1.5rem", borderRadius: "8px", marginBottom: "2rem",
                border: isRunning ? "2px solid #4ade80" : "2px solid #ef4444"
            }}>
                <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center" }}>
                    <div>
                        <h2 style={{ margin: 0 }}>
                            Status: <span style={{ color: isRunning ? "#4ade80" : "#ef4444" }}>
                                {isRunning ? "ONLINE" : "OFFLINE"}
                            </span>
                        </h2>
                        {botProfile && <p style={{ color: "#ccc", margin: "0.5rem 0 0 0" }}>Logged in as: <strong>{botProfile}</strong></p>}
                    </div>
                    {isRunning ? (
                        <button onClick={handleStop} style={{ background: "#ef4444", color: "white", padding: "10px 20px", border: "none", borderRadius: "4px", cursor: "pointer" }}>Stop Bot</button>
                    ) : (
                        <div style={{ display: "flex", gap: "10px" }}>
                            <input type="password" placeholder="Token (Optional)" value={token} onChange={(e) => setToken(e.target.value)} style={{ padding: "10px", borderRadius: "4px", background: "#333", border: "1px solid #555", color: "white" }} />
                            <button onClick={handleStart} style={{ background: "#4ade80", color: "#000", padding: "10px 20px", border: "none", borderRadius: "4px", fontWeight: "bold", cursor: "pointer" }}>Start Bot</button>
                        </div>
                    )}
                </div>
                {statusMessage && <p style={{ marginTop: "1rem", color: "#888" }}>{statusMessage}</p>}
                {error && <p style={{ marginTop: "1rem", color: "#ef4444" }}>Error: {error}</p>}
            </div>

            {isRunning && (
                <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: "1rem", marginBottom: "2rem" }}>

                    <div style={{ background: "#333", padding: "1rem", borderRadius: "8px" }}>
                        <h3 style={{ marginTop: 0 }}>⚔️ Vs Specific Bot</h3>
                        <div style={{ display: "flex", gap: "0.5rem" }}>
                            <input
                                type="text"
                                value={opponent}
                                onChange={(e) => setOpponent(e.target.value)}
                                placeholder="username"
                                style={{ flex: 1, padding: "8px", borderRadius: "4px", border: "none", background: "#222", color: "white" }}
                            />
                            <button onClick={handleBotChallenge} style={{ background: "#3b82f6", color: "white", border: "none", padding: "8px 12px", borderRadius: "4px", cursor: "pointer" }}>Play</button>
                        </div>
                    </div>

                    <div style={{ background: "#333", padding: "1rem", borderRadius: "8px" }}>
                        <h3 style={{ marginTop: 0 }}>🤖 Vs Stockfish AI</h3>
                        <div style={{ display: "flex", gap: "0.5rem", alignItems: "center" }}>
                            <span style={{ color: "#aaa" }}>Level:</span>
                            <select
                                value={stockfishLevel}
                                onChange={(e) => setStockfishLevel(e.target.value)}
                                style={{ padding: "8px", borderRadius: "4px", background: "#222", color: "white", border: "none" }}
                            >
                                {[1,2,3,4,5,6,7,8].map(l => <option key={l} value={l}>{l}</option>)}
                            </select>
                            <button onClick={handleAiChallenge} style={{ flex: 1, background: "#8b5cf6", color: "white", border: "none", padding: "8px", borderRadius: "4px", cursor: "pointer" }}>Play AI</button>
                        </div>
                    </div>

                    <div style={{ background: "#333", padding: "1rem", borderRadius: "8px", gridColumn: "span 2" }}>
                        <h3 style={{ marginTop: 0 }}>🌍 Open Challenge</h3>
                        <p style={{ color: "#888", fontSize: "0.9rem", margin: "0 0 1rem 0" }}>Create a public 3+0 rated game anyone can join.</p>
                        <button onClick={handleOpenChallenge} style={{ width: "100%", background: "#10b981", color: "white", border: "none", padding: "10px", borderRadius: "4px", cursor: "pointer", fontWeight: "bold" }}>
                            Create Open Challenge
                        </button>
                    </div>

                    <div style={{ background: "#333", padding: "1rem", borderRadius: "8px" }}>
                        <h3 style={{ marginTop: 0 }}>🩸 Hunt Weakest Bot</h3>
                        <p style={{ color: "#aaa", fontSize: "0.9rem", margin: "0 0 1rem 0" }}>
                            Scans online bots for the lowest rating and challenges them.
                        </p>
                        <button
                            onClick={handleWeakestChallenge}
                            style={{
                                width: "100%",
                                background: "#dc2626",
                                color: "white",
                                border: "none",
                                padding: "10px",
                                borderRadius: "4px",
                                cursor: "pointer",
                                fontWeight: "bold"
                            }}
                        >
                            Find & Destroy
                        </button>
                    </div>
                </div>
            )}

            {isRunning && (
                <div>
                    <h2 style={{ borderBottom: "1px solid #444", paddingBottom: "0.5rem" }}>Active Games</h2>
                    {activeGames.length === 0 ? (
                        <div style={{ textAlign: "center", padding: "2rem", color: "#888", background: "#222", borderRadius: "8px" }}>Waiting for games...</div>
                    ) : (
                        <div style={{ display: "grid", gap: "1rem", marginTop: "1rem" }}>
                            {activeGames.map((gameId) => (
                                <div key={gameId} style={{ background: "#333", padding: "1rem", borderRadius: "8px", display: "flex", justifyContent: "space-between", alignItems: "center" }}>
                                    <div><strong>Game ID:</strong> {gameId}</div>
                                    <a href={`https://lichess.org/${gameId}`} target="_blank" rel="noopener noreferrer" style={{ background: "#3b82f6", color: "white", textDecoration: "none", padding: "8px 16px", borderRadius: "4px" }}>Watch ↗</a>
                                </div>
                            ))}
                        </div>
                    )}
                </div>
            )}
        </div>
    );
}