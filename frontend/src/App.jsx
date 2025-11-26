// src/App.jsx
import React, { useState } from "react";
import Home from "./pages/Home.jsx";
import Game from "./pages/Game.jsx";

export default function App() {
    const [route, setRoute] = useState("home");
    const [gameId, setGameId] = useState(null);

    const handleStartGame = (id) => {
        setGameId(id);
        setRoute("game");
    };

    return (
        <>
            {route === "home" && <Home onStartGame={handleStartGame} />}
            {route === "game" && <Game gameId={gameId} />}
        </>
    );
}
