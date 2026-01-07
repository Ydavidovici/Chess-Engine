import {useState, useEffect} from "react";
import {health} from "../services/api.js";
import "../../styles/app.css";

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

    return (
        <div className="bg-blue-500 flex flex-col items-center justify-center">
            we live
        </div>
    );
}
