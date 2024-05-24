import React, { useState } from 'react';

const GameControls = ({ gameId, onMakeMove }) => {
    const [move, setMove] = useState('');

    const handleMakeMove = () => {
        onMakeMove(gameId, move);
        setMove('');
    };

    return (
        <div>
            <input
                type="text"
                placeholder="Move"
                value={move}
                onChange={(e) => setMove(e.target.value)}
            />
            <button onClick={handleMakeMove}>Make Move</button>
        </div>
    );
};

export default GameControls;
