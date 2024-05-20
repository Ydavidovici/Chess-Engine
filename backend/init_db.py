# backend/init_db.py

from flask import Flask
from flask_sqlalchemy import SQLAlchemy
import os

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://your_username:your_password@localhost/chess_db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

def init_db():
    with open(os.path.join(os.path.dirname(__file__), 'db/schema.sql'), 'r') as f:
        db.engine.execute(f.read())

if __name__ == '__main__':
    with app.app_context():
        init_db()
    print("Database initialized successfully.")
