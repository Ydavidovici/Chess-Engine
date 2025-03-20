# backend/test_engine.py

import engine

# Create a new Engine object and initialize it
eng = engine.Engine()
eng.initialize()

# Print the initial board state
print(eng.getBoardState())

# Evaluate the initial board state
print("Board evaluation:", eng.evaluateBoard())
