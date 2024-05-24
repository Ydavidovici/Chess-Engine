# backend/game/models.py

from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

class Game(db.Model):
    __tablename__ = 'games'
    game_id = db.Column(db.Integer, primary_key=True)
    start_time = db.Column(db.DateTime, nullable=False)
    end_time = db.Column(db.DateTime)
    result = db.Column(db.String)
    player1_id = db.Column(db.Integer, db.ForeignKey('player_stats.player_id'))
    player2_id = db.Column(db.Integer, db.ForeignKey('player_stats.player_id'))

class GameMove(db.Model):
    __tablename__ = 'game_moves'
    game_id = db.Column(db.Integer, db.ForeignKey('games.game_id'), primary_key=True)
    move_number = db.Column(db.Integer, primary_key=True)
    move_notation = db.Column(db.Text)
