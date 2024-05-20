# backend/user/urls.py

from flask import Blueprint
from .views import register, login, logout

user_bp = Blueprint('user', __name__)

user_bp.route('/register/', methods=['POST'])(register)
user_bp.route('/login/', methods=['POST'])(login)
user_bp.route('/logout/', methods=['POST'])(logout)
