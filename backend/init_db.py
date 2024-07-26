from flask import Flask
from flask_sqlalchemy import SQLAlchemy
import os

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://yaakov:Ydavidovici35@localhost/chess_db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

def init_db():
    schema_path = os.path.join(os.path.dirname(__file__), 'db/schema.sql')
    if not os.path.exists(schema_path):
        print(f"Schema file does not exist at path: {schema_path}")
        return

    try:
        with open(schema_path, 'r') as f:
            db.engine.execute(f.read())
        print("Database initialized successfully.")
    except Exception as e:
        print(f"An error occurred while initializing the database: {e}")

if __name__ == '__main__':
    with app.app_context():
        init_db()
