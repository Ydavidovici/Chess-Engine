import React from 'react';

const Board = ({ board }) => {
    const renderSquare = (piece, index) => {
        return (
            <div
                key={index}
                className={`w-12 h-12 flex items-center justify-center ${index % 2 === 0 ? 'bg-gray-300' : 'bg-gray-600'}`}
            >
                {piece}
            </div>
        );
    };

    return (
        <div className="grid grid-cols-8 gap-0.5">
            {board.split('').map((piece, index) => renderSquare(piece, index))}
        </div>
    );
};

export default Board;
