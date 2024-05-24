# backend/analysis/models.py

from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

class Analysis(db.Model):
    __tablename__ = 'analysis'
    id = db.Column(db.Integer, primary_key=True)
    game_id = db.Column(db.Integer, db.ForeignKey('games.game_id'))
    analysis_data = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=db.func.current_timestamp())
