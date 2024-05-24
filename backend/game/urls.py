# backend/game/urls.py

from flask import Blueprint
from .views import start_game, make_move, game_status

game_bp = Blueprint('game', __name__)

game_bp.route('/start/', methods=['POST'])(start_game)
game_bp.route('/move/', methods=['POST'])(make_move)
game_bp.route('/status/<int:game_id>/', methods=['GET'])(game_status)
