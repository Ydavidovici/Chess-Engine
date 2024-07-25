from lichess_api import LichessAPI
from models import db, PlayerStats, Game, GameMove
from datetime import datetime
import chess_engine  # Import the new chess engine module

class GameManager:
    def __init__(self, token):
        self.lichess_api = LichessAPI(token)
        self.eng = chess_engine.initialize_engine()

    def start_game(self, player1_id, player2_id):
        new_game = Game(
            start_time=datetime.utcnow(),
            player1_id=player1_id,
            player2_id=player2_id
        )
        db.session.add(new_game)
        db.session.commit()

        self.eng = chess_engine.initialize_engine()

        return new_game, chess_engine.get_board_state(self.eng)

    def make_move(self, game_id, move):
        game = Game.query.get(game_id)
        if not game:
            raise ValueError("Game not found")

        # Replay all moves to reach the current state
        moves = GameMove.query.filter_by(game_id=game_id).order_by(GameMove.move_number).all()
        for game_move in moves:
            chess_engine.make_move(self.eng, game_move.move_notation)

        chess_engine.make_move(self.eng, move)
        new_move = GameMove(game_id=game_id, move_number=len(moves) + 1, move_notation=move)
        db.session.add(new_move)
        db.session.commit()

        return chess_engine.get_board_state(self.eng)

    def get_game_status(self, game_id):
        game = Game.query.get(game_id)
        if not game:
            raise ValueError("Game not found")

        # Replay all moves to reach the current state
        moves = GameMove.query.filter_by(game_id=game_id).order_by(GameMove.move_number).all()
        for game_move in moves:
            chess_engine.make_move(self.eng, game_move.move_notation)

        return {
            'game_id': game.game_id,
            'start_time': game.start_time,
            'end_time': game.end_time,
            'result': game.result,
            'board': chess_engine.get_board_state(self.eng),
            'moves': [{'move_number': m.move_number, 'move_notation': m.move_notation} for m in moves]
        }
