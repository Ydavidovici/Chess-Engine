from setuptools import setup, Extension
import os

# Define the extension module
module = Extension(
    'chess_engine',  # The name of the module to be used in Python
    sources=[
        os.path.join('engine', 'src', 'bindings.cpp'),
        os.path.join('engine', 'src', 'board.cpp'),
        os.path.join('engine', 'src', 'engine.cpp'),
        os.path.join('engine', 'src', 'evaluator.cpp'),
        os.path.join('engine', 'src', 'move.cpp'),
        os.path.join('engine', 'src', 'piece.cpp'),
        os.path.join('engine', 'src', 'player.cpp')
    ],
    include_dirs=[os.path.join('engine', 'include')],  # Include directories for header files
    extra_compile_args=[],  # No need to specify any C++ standard flags for MSVC if it's ignored
    language='c++',  # Specify the language
)

# Call setup to build the extension module
setup(
    name='chess_engine',
    version='1.0',
    description='Python bindings for the Chess Engine C++ code',
    ext_modules=[module],
)
