from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

class PlayerStats(db.Model):
    __tablename__ = 'player_stats'
    player_id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False, unique=True)
    wins = db.Column(db.Integer, default=0)
    losses = db.Column(db.Integer, default=0)
    draws = db.Column(db.Integer, default=0)

class Game(db.Model):
    __tablename__ = 'games'
    game_id = db.Column(db.Integer, primary_key=True)
    start_time = db.Column(db.DateTime, nullable=False)
    end_time = db.Column(db.DateTime)
    result = db.Column(db.String)
    player1_name = db.Column(db.String(100), nullable=False)
    player2_name = db.Column(db.String(100), nullable=False)

class GameMove(db.Model):
    __tablename__ = 'game_moves'
    game_id = db.Column(db.Integer, db.ForeignKey('games.game_id'), primary_key=True)
    move_number = db.Column(db.Integer, primary_key=True)
    move_notation = db.Column(db.Text)
