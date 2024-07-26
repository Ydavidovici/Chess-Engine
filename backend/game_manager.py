from lichess_api import LichessAPI
from models import db, PlayerStats, Game, GameMove
from datetime import datetime
import chess_engine  # Import the new chess engine module
import logging

logger = logging.getLogger(__name__)

class GameManager:
    def __init__(self, token):
        self.lichess_api = LichessAPI(token)
        self.eng = chess_engine.initialize_engine()
        logger.debug("Chess engine initialized")

    def start_game(self, player1_name, player2_name):
        try:
            player1 = PlayerStats.query.filter_by(name=player1_name).first()
            if not player1:
                player1 = PlayerStats(name=player1_name)
                db.session.add(player1)
                db.session.commit()
                logger.debug(f"Player 1 created: {player1}")

            player2 = PlayerStats.query.filter_by(name=player2_name).first()
            if not player2:
                player2 = PlayerStats(name=player2_name)
                db.session.add(player2)
                db.session.commit()
                logger.debug(f"Player 2 created: {player2}")

            new_game = Game(
                start_time=datetime.utcnow(),
                player1_name=player1_name,
                player2_name=player2_name
            )
            db.session.add(new_game)
            db.session.commit()
            logger.debug(f"New game created: {new_game}")

            self.eng = chess_engine.initialize_engine()
            logger.debug("Chess engine re-initialized")

            # Use a hard-coded starting FEN
            initial_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            logger.debug(f"Initial FEN (hard-coded): {initial_fen}")

            return new_game, initial_fen
        except Exception as e:
            logger.exception("Exception in start_game")
            raise e

    def make_move(self, game_id, move):
        try:
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
            logger.debug(f"Move made: {new_move}")

            return chess_engine.get_board_state(self.eng)
        except ValueError as e:
            logger.exception("Error making move")
            raise e
        except Exception as e:
            logger.exception("Exception in make_move")
            raise e

    def get_game_status(self, game_id):
        try:
            game = Game.query.get(game_id)
            if not game:
                raise ValueError("Game not found")

            # Replay all moves to reach the current state
            moves = GameMove.query.filter_by(game_id=game_id).order_by(GameMove.move_number).all()
            for game_move in moves:
                chess_engine.make_move(self.eng, game_move.move_notation)

            status = {
                'game_id': game.game_id,
                'start_time': game.start_time,
                'end_time': game.end_time,
                'result': game.result,
                'player1_name': game.player1_name,
                'player2_name': game.player2_name,
                'board': chess_engine.get_board_state(self.eng),
                'moves': [{'move_number': m.move_number, 'move_notation': m.move_notation} for m in moves]
            }
            logger.debug(f"Game status: {status}")
            return status
        except ValueError as e:
            logger.exception("Error fetching game status")
            raise e
        except Exception as e:
            logger.exception("Exception in get_game_status")
            raise e
