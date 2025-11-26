// src/pages/Home.jsx
import {useState, useEffect} from "react";
import ChessboardComponent from "../components/Chessboard.jsx";
import Sidebar from "../components/Sidebar.jsx";
import {health, startGame} from "../services/api.js";

export default function Home() {
    const [player1, setPlayer1] = useState("");
    const [player2, setPlayer2] = useState("");
    const [gameId, setGameId] = useState(null);
    const [gameStatus, setGameStatus] = useState("Not Started");


    useEffect(() => {
        const runHealthCheck = async () => {
            try {
                const data = await health();
                console.log("API returned:", data);
            } catch (e) {
                console.error("Health check failed:", e);
            }
        };
        runHealthCheck();
    });

    const handleStartGame = async () => {
        try {
            const data = await startGame({
                player1Id: player1,
                player2Id: player2,
            });

            const newGameId = data.game_id;
            setGameId(newGameId);
            setGameStatus("In Progress");

        } catch (error) {
            console.error("Error starting game:", error);
        }
    };

    return (
        <div className="container mx-auto flex">
            <div className="w-3/4 p-4">
                <h1 className="text-3xl font-bold mb-4">Start a New Game</h1>

                <input
                    type="text"
                    placeholder="Player 1 ID"
                    value={player1}
                    onChange={(e) => setPlayer1(e.target.value)}
                    className="border p-2 m-2 w-full"
                />

                <input
                    type="text"
                    placeholder="Player 2 ID"
                    value={player2}
                    onChange={(e) => setPlayer2(e.target.value)}
                    className="border p-2 m-2 w-full"
                />

                <button
                    onClick={handleStartGame}
                    className="bg-blue-500 text-white p-2 m-2"
                >
                    Start Game
                </button>

                <ChessboardComponent/>
            </div>

            <Sidebar
                player1={player1}
                player2={player2}
                gameId={gameId}
                gameStatus={gameStatus}
            />
        </div>
    );
}
