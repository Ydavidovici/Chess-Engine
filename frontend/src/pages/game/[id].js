import { useState, useEffect } from 'react';
import axios from 'axios';
import { useRouter } from 'next/router';
import ChessboardComponent from '../../components/Chessboard';

export default function Game() {
    const router = useRouter();
    const { id } = router.query;
    const [board, setBoard] = useState('');

    useEffect(() => {
        if (id) {
            getGameStatus();
        }
    }, [id]);

    const getGameStatus = async () => {
        try {
            const response = await axios.get(`http://127.0.0.1:5000/game_status/${id}`);
            setBoard(response.data.board);
        } catch (error) {
            console.error('Error fetching game status:', error);
        }
    };

    return (
        <div className="container mx-auto">
            <h1 className="text-3xl font-bold">Game {id}</h1>
            <ChessboardComponent />
        </div>
    );
}
