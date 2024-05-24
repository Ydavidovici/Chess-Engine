import { useState, useEffect } from 'react';
import { useRouter } from 'next/router';
import { getGameStatus, makeMove } from '../../services/api';
import Board from '../../components/Board';
import GameControls from '../../components/GameControls';

const Game = () => {
    const router = useRouter();
    const { id } = router.query;
    const [game, setGame] = useState(null);

    useEffect(() => {
        if (id) {
            fetchGameStatus(id);
        }
    }, [id]);

    const fetchGameStatus = async (gameId) => {
        const gameStatus = await getGameStatus(gameId);
        setGame(gameStatus);
    };

    const handleMakeMove = async (gameId, move) => {
        const gameState = await makeMove(gameId, move);
        setGame({ ...game, board: gameState.board });
    };

    if (!game) return <div>Loading...</div>;

    return (
        <div>
            <h1>Game ID: {game.game_id}</h1>
            <Board board={game.board} />
            <GameControls gameId={game.game_id} onMakeMove={handleMakeMove} />
        </div>
    );
};

export default Game;
