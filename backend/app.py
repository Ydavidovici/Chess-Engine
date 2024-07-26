import sys
import os
from flask import Flask, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
from models import db, PlayerStats, Game, GameMove
from game_manager import GameManager
from datetime import datetime
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Determine the path to chess_engine.so and add it to the Python path
engine_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/backend/engine'))
print(f"Adding {engine_path} to sys.path")  # Debug print
sys.path.append(engine_path)
print(f"sys.path: {sys.path}")  # Debug print to see the paths being added

# Set the LD_LIBRARY_PATH environment variable
os.environ['LD_LIBRARY_PATH'] = engine_path + ':' + os.environ.get('LD_LIBRARY_PATH', '')
print(f"LD_LIBRARY_PATH: {os.environ['LD_LIBRARY_PATH']}")  # Debug print

try:
    import chess_engine  # Import the new chess engine module
    print("chess_engine module loaded successfully")  # Debug print to confirm module loading
except ModuleNotFoundError as e:
    print(f"Error loading chess_engine: {e}")  # Debug print to capture the error

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URI')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db.init_app(app)

lichess_token = os.getenv('LICHESS_TOKEN')
game_manager = GameManager(lichess_token)

@app.route('/start_game', methods=['POST'])
def start_game():
    data = request.get_json()
    player1_id = data.get('player1_id')
    player2_id = data.get('player2_id')

    if not player1_id or not player2_id:
        return jsonify({'error': 'Player IDs are required'}), 400

    try:
        new_game, initial_fen = game_manager.start_game(player1_id, player2_id)
        return jsonify({
            'game_id': new_game.game_id,
            'fen': initial_fen
        }), 201
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/make_move', methods=['POST'])
def make_move():
    data = request.get_json()
    game_id = data.get('game_id')
    move = data.get('move')

    if not game_id or not move:
        return jsonify({'error': 'Game ID and move are required'}), 400

    try:
        new_fen = game_manager.make_move(game_id, move)
        return jsonify({'fen': new_fen}), 200
    except ValueError as e:
        return jsonify({'error': str(e)}), 404
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/game_status/<int:game_id>', methods=['GET'])
def game_status(game_id):
    try:
        status = game_manager.get_game_status(game_id)
        return jsonify(status), 200
    except ValueError as e:
        return jsonify({'error': str(e)}), 404
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
