# backend/game/views.py

from flask import jsonify, request
from .models import db, Game, GameMove
from game_manager import GameManager
from datetime import datetime

game_manager = GameManager('your_lichess_token')

def start_game():
    data = request.get_json()
    player1_id = data.get('player1_id')
    player2_id = data.get('player2_id')

    new_game = game_manager.start_game(player1_id, player2_id)

    return jsonify({
        'game_id': new_game.game_id,
        'board': game_manager.eng.getBoardState()
    }), 201

def make_move():
    data = request.get_json()
    game_id = data.get('game_id')
    move = data.get('move')

    try:
        board_state = game_manager.make_move(game_id, move)
    except ValueError as e:
        return jsonify({'error': str(e)}), 404

    return jsonify({'board': board_state}), 200

def game_status(game_id):
    try:
        status = game_manager.get_game_status(game_id)
    except ValueError as e:
        return jsonify({'error': str(e)}), 404

    return jsonify(status), 200
