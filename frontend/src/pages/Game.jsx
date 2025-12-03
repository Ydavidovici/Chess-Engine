import {useEffect} from "react";
import {bestMove} from "../services/api.js";

export default function Game({ gameId }) {

    useEffect(() => {
        getBestMove();
    });

    const getBestMove = async () => {
        try {
            const data = await bestMove({
                fen: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            });
            console.log("Best move:", data.bestMove);
        } catch (err) {
            console.error("Best move request failed:", err);
        }
    };


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
        <div className="p-4">
            <h1 className="text-xl font-bold">Game ID: {gameId}</h1>
            Game page!
        </div>
    );
}
