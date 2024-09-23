import chess_engine

def test_chess_engine():
    print("Initializing engine")
    engine = chess_engine.initialize_engine()
    print("Engine initialized in Python")

    print("Getting board state")
    initial_board_state = chess_engine.get_board_state_engine(engine)
    print(f"Initial board state:\n{initial_board_state}")

    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    print(f"Setting board from FEN: {fen}")
    chess_engine.set_board_from_fen(engine, fen)

    board_state_after_fen = chess_engine.get_board_state_engine(engine)
    print(f"Board state after setting FEN:\n{board_state_after_fen}")

    move = chess_engine.create_move("e2e4")
    print(f"Move created: {chess_engine.get_move(move)}")

    print(f"Making move: {chess_engine.get_move(move)}")
    chess_engine.make_move(engine, move)

    board_state_after_move = chess_engine.get_board_state_engine(engine)
    print(f"Board state after move e2e4:\n{board_state_after_move}")

    print("Evaluating board")
    board_evaluation = chess_engine.evaluate_board(engine)
    print(f"Board evaluation: {board_evaluation}")

if __name__ == "__main__":
    test_chess_engine()
