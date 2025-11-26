import axios from 'axios';

const API_URL = import.meta.env.VITE_API_URL ?? "http://localhost:8000/api";

export const startGame = async (player1_id, player2_id) => {
    const response = await axios.post(`${API_URL}/start_game`, { player1_id, player2_id });
    return response.data;
};

export const makeMove = async (game_id, move) => {
    const response = await axios.post(`${API_URL}/make_move`, { game_id, move });
    return response.data;
};

export const getGameStatus = async (game_id) => {
    const response = await axios.get(`${API_URL}/game_status/${game_id}`);
    return response.data;
};

export const health = async () => {
    try {
        console.log(`'url', ${API_URL}`)
        const {data} = await axios.get(`${API_URL}/health`);
        return data;
    } catch (err) {
        console.error('health check failed:', err);
    }
};
