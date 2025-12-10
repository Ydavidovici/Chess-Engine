import axios from 'axios';

const API_URL = import.meta.env.API_URL ?? "http://localhost:8000";

export async function apiRequest(path, options = {}) {
    const res = await fetch(`${API_URL}${path}`, {
        headers: { "Content-Type": "application/json", ...(options.headers || {}) },
        ...options,
    });
    if (!res.ok) {
        const text = await res.text();
        throw new Error(`HTTP ${res.status}: ${text}`);
    }
    return res.json();
}

export const startGame = async (player1_id, player2_id) => {
    const { data } = await axios.post(`${API_URL}/api/start_game`, {
        player1_id,
        player2_id,
    });
    return data;
};

export const makeMove = async (fen, move) => {
    const { data } = await axios.post(`${API_URL}/api/engine/make-move`, {fen, move});
    return data;
};

export const getGameStatus = async (game_id) => {
    const { data } = await axios.get(`${API_URL}/api/game-status/${game_id}`);
    return data;
};

export const health = async () => {
    try {
        console.log("API_URL:", API_URL);
        const { data } = await axios.get(`${API_URL}/api/health`);
        return data;
    } catch (err) {
        console.error("health check failed:", err);
        throw err;
    }
};

export const bestMove = async ({ fen, moves, depth, movetime }) => {
    const { data } = await axios.post(`${API_URL}/api/engine/best-move`, {fen});
    return data;
};

export const printPosition = async (fen) => {
    const {data} = await axios.post(`${API_URL}/api/engine/print-position`, {fen})
    return data;
}

