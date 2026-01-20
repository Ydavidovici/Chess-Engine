import React from "react";
import { Routes, Route } from "react-router-dom";
import Default from "./layouts/Default.jsx";
import Home from "./pages/Home.jsx";
import Game from "./pages/Game.jsx";
import BenchmarkControl from "./pages/BenchmarkControl.jsx";

export default function App() {
    return (
        <Routes>
            <Route element={<Default />}>
                <Route path="/" element={<Home/>} />
                <Route path="/game" element={<Game/>} />
                <Route path="/benchmark" element={<BenchmarkControl/>} />
            </Route>
        </Routes>
    );
}
