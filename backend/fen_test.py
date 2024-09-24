import sys
import os
import logging
from flask import Flask, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
from datetime import datetime
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Determine the path to chess_engine.so and add it to the Python path
engine_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/backend/engine'))
print(f"Adding {engine_path} to sys.path")  # Debug print
sys.path.append(engine_path)
print(f"sys.path after appending engine_path: {sys.path}")  # Debug print to see the paths being added

# Set the LD_LIBRARY_PATH environment variable
os.environ['LD_LIBRARY_PATH'] = engine_path + ':' + os.environ.get('LD_LIBRARY_PATH', '')
print(f"LD_LIBRARY_PATH: {os.environ['LD_LIBRARY_PATH']}")  # Debug print

try:
    import chess_engine  # Import the new chess engine module
    print("chess_engine module loaded successfully")  # Debug print to confirm module loading
except ModuleNotFoundError as e:
    print(f"Error loading chess_engine: {e}")  # Debug print to capture the error
    sys.exit(1)  # Exit if the module is not found

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Configure logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

# In-memory storage for FEN strings for simplicity
fen_store = {}

@app.route('/set_fen', methods=['POST'])
def set_fen():
    logger.debug("Received request to set FEN")
    try:
        data = request.get_json()
        game_id = data.get('game_id')
        fen = data.get('fen')
        logger.debug(f"Game ID: {game_id}, FEN: {fen}")

        if not game_id or not fen:
            logger.error("Game ID and FEN are required")
            return jsonify({'error': 'Game ID and FEN are required'}), 400

        fen_store[game_id] = fen
        logger.debug(f"Stored FEN for game_id={game_id}")
        return jsonify({'message': 'FEN set successfully'}), 200
    except Exception as e:
        logger.exception("Error setting FEN")
        return jsonify({'error': str(e)}), 500

@app.route('/get_fen/<int:game_id>', methods=['GET'])
def get_fen(game_id):
    logger.debug(f"Received request to get FEN for game_id={game_id}")
    try:
        fen = fen_store.get(game_id)
        if not fen:
            logger.error(f"No FEN found for game_id={game_id}")
            return jsonify({'error': 'FEN not found'}), 404
        logger.debug(f"Retrieved FEN for game_id={game_id}: {fen}")
        return jsonify({'fen': fen}), 200
    except Exception as e:
        logger.exception("Error getting FEN")
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    logger.debug("Starting Flask application")
    app.run(debug=True)
