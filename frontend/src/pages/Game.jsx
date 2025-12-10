import {useEffect} from "react";
import {bestMove, makeMove, printPosition} from "../services/api.js";

export default function Game({ gameId }) {

    useEffect(() => {
        // getBestMove();
        // printPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
        // makeMove("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "a2a3")
        // makeMove("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1", "e7e5")
        // makeMove( "rnbqkbnr/pppp1ppp/8/4p3/8/P7/1PPPPPPP/RNBQKBNR w KQkq e6 0 2", "g1f3")
        // makeMove( "rnbqkbnr/pppp1ppp/8/4p3/8/P4N2/1PPPPPPP/RNBQKB1R b KQkq - 1 2", "g8f1")
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
