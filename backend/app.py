import sys
import os
import logging
from flask import Flask, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
from models import db, PlayerStats, Game, GameMove
from game_manager import GameManager
from datetime import datetime
from dotenv import load_dotenv
from init_db import init_db  # Import the init_db function

# Load environment variables from .env file
load_dotenv()

# Determine the path to chess_engine.so and add it to the Python path
engine_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/backend/engine'))
print(f"Adding {engine_path} to sys.path")  # Debug print
sys.path.append(engine_path)
print(f"sys.path after appending engine_path: {sys.path}")  # Debug print to see the paths being added

# Set the LD_LIBRARY_PATH environment variable
os.environ['LD_LIBRARY_PATH'] = engine_path + ':' + os.environ.get('LD_LIBRARY_PATH', '')
print(f"LD_LIBRARY_PATH: {os.environ['LD_LIBRARY_PATH']}")  # Debug print

try:
    import chess_engine  # Import the new chess engine module
    print("chess_engine module loaded successfully")  # Debug print to confirm module loading
except ModuleNotFoundError as e:
    print(f"Error loading chess_engine: {e}")  # Debug print to capture the error
    sys.exit(1)  # Exit if the module is not found

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URI').replace('postgresql', 'postgresql+pg8000')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db.init_app(app)

lichess_token = os.getenv('LICHESS_TOKEN')
game_manager = GameManager(lichess_token)

# Configure logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

# Initialize the database if needed
try:
    with app.app_context():
        logger.debug("Initializing the database")
        init_db()
except Exception as e:
    logger.exception("Exception occurred during database initialization")

@app.route('/start_game', methods=['POST'])
def start_game():
    data = request.get_json()
    player1_name = data.get('player1_name')
    player2_name = data.get('player2_name')

    if not player1_name or not player2_name:
        return jsonify({'error': 'Player names are required'}), 400

    try:
        new_game, initial_fen = game_manager.start_game(player1_name, player2_name)
        return jsonify({
            'game_id': new_game.game_id,
            'fen': initial_fen
        }), 201
    except Exception as e:
        logger.exception("Error starting game")
        return jsonify({'error': str(e)}), 500


@app.route('/make_move', methods=['POST'])
def make_move():
    logger.debug("Received request to make move")
    try:
        data = request.get_json()
        logger.debug(f"Request data: {data}")
        game_id = data.get('game_id')
        move = data.get('move')
        logger.debug(f"Game ID: {game_id}, Move: {move}")

        if not game_id or not move:
            logger.error("Game ID and move are required")
            return jsonify({'error': 'Game ID and move are required'}), 400

        new_fen = game_manager.make_move(game_id, move)
        logger.debug(f"Move made: new_fen={new_fen}")
        return jsonify({'fen': new_fen}), 200
    except ValueError as e:
        logger.exception("Error making move")
        return jsonify({'error': str(e)}), 404
    except Exception as e:
        logger.exception("Error making move")
        return jsonify({'error': str(e)}), 500

@app.route('/game_status/<int:game_id>', methods=['GET'])
def game_status(game_id):
    logger.debug(f"Received request to get game status for game_id={game_id}")
    try:
        status = game_manager.get_game_status(game_id)
        logger.debug(f"Game status: {status}")
        return jsonify(status), 200
    except ValueError as e:
        logger.exception("Error fetching game status")
        return jsonify({'error': str(e)}), 404
    except Exception as e:
        logger.exception("Error fetching game status")
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    logger.debug("Starting Flask application")
    app.run(debug=True)
