import React, { useState } from 'react';
import { Chessboard } from 'react-chessboard';

const ChessboardComponent = () => {
    const [position, setPosition] = useState('start');

    const onDrop = (sourceSquare, targetSquare) => {
        console.log(`Move from ${sourceSquare} to ${targetSquare}`);
        // Handle the move logic here
        // Update the position state if the move is valid
    };

    return (
        <div>
            <Chessboard
                position={position}
                onPieceDrop={(sourceSquare, targetSquare) => onDrop(sourceSquare, targetSquare)}
            />
        </div>
    );
};

export default ChessboardComponent;
