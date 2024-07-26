import sys
import os

# Determine the path to chess_engine.so and add it to the Python path
engine_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/backend/engine'))
print(f"Adding {engine_path} to sys.path")
sys.path.append(engine_path)
print(f"sys.path after appending engine_path: {sys.path}")

# Set the LD_LIBRARY_PATH environment variable
os.environ['LD_LIBRARY_PATH'] = engine_path + ':' + os.environ.get('LD_LIBRARY_PATH', '')
print(f"LD_LIBRARY_PATH: {os.environ['LD_LIBRARY_PATH']}")

try:
    import chess_engine  # Import the new chess engine module
    print("chess_engine module loaded successfully")
except ModuleNotFoundError as e:
    print(f"Error loading chess_engine: {e}")
