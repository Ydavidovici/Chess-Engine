#!/usr/bin/env python3
import random
import chess
import chess.engine
from pyengine import Engine, Move, Color, MoveType

def square_to_index(sq: int) -> int:
    """
    Convert python-chess square index (0-63) to our engine's index (rank*8 + file).
    python-chess uses a1=0, h8=63 with same mapping.
    """
    file = chess.square_file(sq)
    rank = chess.square_rank(sq)
    return rank * 8 + file

def generate_random_game(num_moves: int = 40, seed: int = None):
    """
    Play a random sequence of moves on both python-chess and our engine.
    Returns the final python-chess Board and our Engine instance.
    """
    if seed is not None:
        random.seed(seed)

    board = chess.Board()
    engine = Engine()
    engine.new_game()

    for _ in range(num_moves):
        if board.is_game_over():
            break
        # Pick a random legal move
        mv = random.choice(list(board.legal_moves))
        board.push(mv)

        # Convert to our Move
        m = Move()
        m.start = square_to_index(mv.from_square)
        m.end   = square_to_index(mv.to_square)
        if mv.promotion:
            m.type = MoveType.PROMOTION
            # promotion piece from chess.Piece.symbol, e.g. 'q'
            m.promo = mv.promotion.symbol().upper()

        # engine color is the side who just moved in python-chess
        color = Color.WHITE if board.turn == chess.BLACK else Color.BLACK
        if not engine.make_move(m, color):
            print(f"[Engine] rejected move {mv.uci()} at ply {board.fullmove_number}")
            break

    return board, engine

def main():
    # 1) Generate a reproducible random game
    board, engine = generate_random_game(num_moves=40, seed=123)

    print("Final python-chess FEN:", board.fen())
    print("\nEngine's board:")
    engine.print_board()

    our_eval = engine.evaluate_board()
    print("\nEngine eval:", our_eval)

    # 2) Ask Stockfish for its evaluation at depth 3
    with chess.engine.SimpleEngine.popen_uci("stockfish") as sf:
        info = sf.analyse(board, limit=chess.engine.Limit(depth=3))
        sf_score = info["score"].white().score(mate_score=100000)
    print("Stockfish eval (d3):", sf_score)

if __name__ == "__main__":
    main()