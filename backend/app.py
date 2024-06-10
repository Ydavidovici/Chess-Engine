from flask import Flask, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from models import db, PlayerStats, Game, GameMove
from game_manager import GameManager
from datetime import datetime
import os
import sys
print(sys.path)

app = Flask(__name__)

# Load database URI and Lichess token from environment variables
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URI', 'postgresql://yaakov:Ydavidovici35@localhost/chess_db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db.init_app(app)

lichess_token = os.getenv('LICHESS_TOKEN', 'your_lichess_token')
game_manager = GameManager(lichess_token)

@app.route('/start_game', methods=['POST'])
def start_game():
    data = request.get_json()
    player1_id = data.get('player1_id')
    player2_id = data.get('player2_id')

    if not player1_id or not player2_id:
        return jsonify({'error': 'Player IDs are required'}), 400

    try:
        new_game = game_manager.start_game(player1_id, player2_id)
        return jsonify({
            'game_id': new_game.game_id,
            'board': game_manager.eng.getBoardState()
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
        board_state = game_manager.make_move(game_id, move)
        return jsonify({'board': board_state}), 200
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
