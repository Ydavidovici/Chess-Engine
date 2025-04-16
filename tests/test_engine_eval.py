import sys
import os
import chess
import chess.engine

# Add backend directory to the Python path
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', 'backend'))
sys.path.insert(0, parent_dir)

from engine import Engine, Move, Color

# FEN for our middlegame test
MIDDLEGAME_FEN = "r1bq1rk1/ppp2ppp/2n2n2/3pp3/3PP3/2N2N2/PPP2PPP/R1BQ1RK1 w - - 0 1"

def test_engine_evaluation():
    engine = Engine()

    # Initialize a fresh chess board
    engine.initialize()
    print("Initial board:")
    print(engine.get_board_state())

    # Make a basic move: White pawn e2 to e4
    move = Move(start=12, end=28)  # e2 (square 12) to e4 (square 28)
    engine.make_move(move, Color.WHITE)

    print("Board after 1.e4:")
    print(engine.get_board_state())

    # Evaluate the board state
    evaluation = engine.evaluate_board()
    print("Board evaluation after 1.e4:", evaluation)

    # Test removing a black pawn from the board to see evaluation change
    engine.board.black_pawns &= ~(1 << 52)  # Remove pawn from e7

    print("Board after removing black pawn e7:")
    print(engine.get_board_state())

    evaluation_after_removal = engine.evaluate_board()
    print("Board evaluation after removing black pawn:", evaluation_after_removal)


def test_middlegame():
    engine = Engine()
    engine.board.load_fen(MIDDLEGAME_FEN)

    print("\nMiddlegame position:")
    print(engine.get_board_state())

    eval_score = engine.evaluate_board()
    print("Evaluation:", eval_score)

    best_move = engine.find_best_move(max_depth=3, color=Color.WHITE)
    print("Best move (White):", best_move.get_move_str() if best_move else "None")


def compare_with_stockfish(fen: str, depth: int):
    # Prepare python-chess board
    board = chess.Board(fen)

    # Our engine
    eng = Engine()
    eng.board.load_fen(fen)
    our_move = eng.find_best_move(max_depth=depth, color=Color.WHITE)
    our_eval = eng.evaluate_board()

    # Stockfish analysis
    with chess.engine.SimpleEngine.popen_uci("stockfish") as sf:
        info = sf.analyse(board, limit=chess.engine.Limit(depth=depth))
        sf_move = info["pv"][0]
        sf_eval = info["score"].white().score(mate_score=100000)

    # Print results
    print(f"\n=== Depth {depth} Comparison ===")
    print(f"Your engine:  move = {our_move.get_move_str() if our_move else 'None'}, eval = {our_eval}")
    print(f"Stockfish:    move = {sf_move.uci()}, eval = {sf_eval}")


if __name__ == "__main__":
    # 1) Basic smoke tests
    test_engine_evaluation()
    test_middlegame()

    # 2) Engine vs Stockfish at depth 3
    compare_with_stockfish(MIDDLEGAME_FEN, depth=3)