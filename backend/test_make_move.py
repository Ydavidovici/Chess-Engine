from models import db, PlayerStats, Game, GameMove
from datetime import datetime, timezone
from mock_chess_engine import mock_chess_engine  # Import the mock chess engine instance
import logging

logger = logging.getLogger(__name__)

class GameManager:
    def __init__(self, token):
        self.eng = mock_chess_engine
        logger.debug("Chess engine initialized")

    def make_move(self, game_id, move):
        try:
            logger.debug("Fetching game from database")
            game = Game.query.get(game_id)
            if not game:
                logger.error("Game not found")
                raise ValueError("Game not found")

            logger.debug("Replaying all moves to reach the current state")
            moves = GameMove.query.filter_by(game_id=game_id).order_by(GameMove.move_number).all()
            for game_move in moves:
                self.eng.make_move(game_move.move_notation)
                logger.debug(f"Replayed move: {game_move.move_notation}")

            logger.debug(f"Making move: {move}")
            self.eng.make_move(move)
            logger.debug("Move executed on engine")

            new_move = GameMove(game_id=game_id, move_number=len(moves) + 1, move_notation=move)
            logger.debug(f"New move created: {new_move}")
            db.session.add(new_move)
            logger.debug("New move added to session")
            db.session.commit()
            logger.debug("Session committed")

            board_state = self.eng.get_board_state()
            logger.debug(f"Board state after move: {board_state}")
            if not board_state:
                logger.error("Received empty FEN string after making move")
                board_state = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
                logger.debug("Board state was empty, reset to initial FEN")

            return board_state
        except ValueError as e:
            logger.exception("Error making move")
            raise e
        except Exception as e:
            logger.exception("Exception in make_move")
            raise e

def test_make_move():
    # Setup test environment
    import os
    from flask import Flask
    from flask_sqlalchemy import SQLAlchemy
    from dotenv import load_dotenv

    # Load environment variables
    load_dotenv()

    app = Flask(__name__)
    app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URI').replace('postgresql', 'postgresql+pg8000')
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
    db = SQLAlchemy(app)

    # Initialize database
    with app.app_context():
        db.create_all()

        # Create or get test data
        player1 = PlayerStats.query.filter_by(name="Alice").first()
        if not player1:
            player1 = PlayerStats(name="Alice")
            db.session.add(player1)

        player2 = PlayerStats.query.filter_by(name="Bob").first()
        if not player2:
            player2 = PlayerStats(name="Bob")
            db.session.add(player2)

        db.session.commit()

        new_game = Game(
            start_time=datetime.now(timezone.utc),
            player1_name="Alice",
            player2_name="Bob"
        )
        db.session.add(new_game)
        db.session.commit()

        game_id = new_game.game_id

        # Initialize GameManager and make a move
        game_manager = GameManager(token=None)
        try:
            fen = game_manager.make_move(game_id, "e2e4")
            print(f"FEN after move: {fen}")
        except Exception as e:
            print(f"Exception: {e}")

if __name__ == '__main__':
    test_make_move()
