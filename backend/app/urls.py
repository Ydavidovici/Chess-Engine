# backend/app/urls.py

from flask import Blueprint
from analysis.urls import analysis_bp
from game.urls import game_bp
from user.urls import user_bp

main_bp = Blueprint('main', __name__)

main_bp.register_blueprint(analysis_bp, url_prefix='/analysis')
main_bp.register_blueprint(game_bp, url_prefix='/game')
main_bp.register_blueprint(user_bp, url_prefix='/user')
