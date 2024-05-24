# backend/app/__init__.py

from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager
from analysis.urls import analysis_bp
from game.urls import game_bp
from user.urls import user_bp

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://your_username:your_password@localhost/chess_db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SECRET_KEY'] = 'your_secret_key'

db = SQLAlchemy(app)
login_manager = LoginManager(app)

login_manager.login_view = 'user.login'

@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))

# Register blueprints
app.register_blueprint(analysis_bp, url_prefix='/analysis')
app.register_blueprint(game_bp, url_prefix='/game')
app.register_blueprint(user_bp, url_prefix='/user')

with app.app_context():
    db.create_all()
