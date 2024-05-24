# backend/analysis/urls.py

from flask import Blueprint
from .views import analyze_game

analysis_bp = Blueprint('analysis', __name__)

analysis_bp.route('/analyze/<int:game_id>', methods=['GET'])(analyze_game)
