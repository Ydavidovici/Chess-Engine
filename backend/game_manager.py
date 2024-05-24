# backend/game_manager.py

import engine  # Import the C++ engine bindings
from lichess_api import LichessAPI
from models import db, PlayerStats, Game, GameMove
from datetime import datetime

class GameManager:
    def __init__(self, token):
        self.lichess_api = LichessAPI(token)
        self.eng = engine.Engine()
        self.eng.initialize()

    def start_game(self, player1_id, player2_id):
        new_game = Game(
            start_time=datetime.utcnow(),
            player1_id=player1_id,
            player2_id=player2_id
        )
        db.session.add(new_game)
        db.session.commit()

        self.eng.initialize()

        return new_game

    def make_move(self, game_id, move):
        game = Game.query.get(game_id)
        if not game:
            raise ValueError("Game not found")

        # Replay all moves to reach the current state
        moves = GameMove.query.filter_by(game_id=game_id).order_by(GameMove.move_number).all()
        for game_move in moves:
            self.eng.makeMove(game_move.move_notation)

        self.eng.makeMove(move)
        new_move = GameMove(game_id=game_id, move_number=len(moves) + 1, move_notation=move)
        db.session.add(new_move)
        db.session.commit()

        return self.eng.getBoardState()

    def get_game_status(self, game_id):
        game = Game.query.get(game_id)
        if not game:
            raise ValueError("Game not found")

        # Replay all moves to reach the current state
        moves = GameMove.query.filter_by(game_id=game_id).order_by(GameMove.move_number).all()
        for game_move in moves:
            self.eng.makeMove(game_move.move_notation)

        return {
            'game_id': game.game_id,
            'start_time': game.start_time,
            'end_time': game.end_time,
            'result': game.result,
            'board': self.eng.getBoardState(),
            'moves': [{'move_number': m.move_number, 'move_notation': m.move_notation} for m in moves]
        }
