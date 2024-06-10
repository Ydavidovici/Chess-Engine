# backend/engine/__init__.py
import os
import sys

# Ensure the path to the DLL is included
sys.path.append(os.path.dirname(__file__))

from .engine import Engine
