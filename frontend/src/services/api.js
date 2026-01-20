import axios from "axios";

const BASE_URL = import.meta.env.API_URL ?? "http://localhost:8000";

/**
 * Generic API Request Handler
 * @param {string} endpoint - The URI path (e.g., '/api/health')
 * @param {object} config - Configuration object {method, data, headers, ...}
 */
export const request = async (endpoint, {method = "GET", data, headers = {}, ...customConfig} = {}) => {
    try {
        const response = await axios({
            url: `${BASE_URL}${endpoint}`,
            method,
            data,
            headers: {
                "Content-Type": "application/json",
                ...headers,
            },
            ...customConfig,
        });

        return response.data;
    } catch (error) {
        console.error(`API Request failed: ${method} ${endpoint}`, error);
        throw error;
    }
};


export const startGame = (player1_id, player2_id) =>
    request("/api/start_game", {
        method: "POST",
        data: {player1_id, player2_id},
    });

export const makeMove = (fen, move) =>
    request("/api/engine/make-move", {
        method: "POST",
        data: {fen, move},
    });

export const getGameStatus = (game_id) =>
    request(`/api/game-status/${game_id}`, {
        method: "GET",
    });

export const health = () =>
    request("/api/health", {
        method: "GET",
    });

export const bestMove = ({fen, moves, depth, movetime}) =>
    request("/api/engine/best-move", {
        method: "POST",
        data: {fen, moves, depth, movetime},
    });

export const printPosition = (fen) =>
    request("/api/engine/print-position", {
        method: "POST",
        data: {fen},
    });

export const runBenchmark = (options) =>
    request("/api/engine/bench", {
        method: "POST",
        data: options
    });