import { useState } from 'react';
import { startGame } from '../services/api';
import PlayerForm from '../components/PlayerForm';
import { useRouter } from 'next/router';

export default function Home() {
    const router = useRouter();

    const handleStartGame = async (player1Id, player2Id) => {
        const game = await startGame(player1Id, player2Id);
        router.push(`/game/${game.game_id}`);
    };

    return (
        <div>
            <h1>Chess Game</h1>
            <PlayerForm onStartGame={handleStartGame} />
        </div>
    );
}
