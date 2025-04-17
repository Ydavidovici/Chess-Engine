import pytest
from backend.engine import Engine, Move, Color

# FEN for our middlegame test
MIDDLEGAME_FEN = "r1bq1rk1/ppp2ppp/2n2n2/3pp3/3PP3/2N2N2/PPP2PPP/R1BQ1RK1 w - - 0 1"

def test_engine_evaluation_smoke():
    engine = Engine()
    engine.initialize()
    # Initial position should be the standard chess setup
    state = engine.get_board_state().splitlines()[0]
    assert state.startswith("r n b q k b n r")
    # simple evaluation sanity check (should be 0 material balance)
    assert engine.evaluate_board() == 0

def test_pawn_push_and_capture_eval():
    engine = Engine()
    engine.initialize()
    # e2e4
    m = Move(start=12, end=28)  # e2->e4
    assert engine.make_move(m, Color.WHITE)
    ev1 = engine.evaluate_board()
    # remove black pawn on e7
    engine.board.black_pawns &= ~(1 << 52)
    ev2 = engine.evaluate_board()
    # capturing a pawn should increase evaluation
    assert ev2 > ev1

def test_middlegame_and_best_move():
    engine = Engine()
    engine.board.load_fen(MIDDLEGAME_FEN)
    # evaluation is some finite integer
    ev = engine.evaluate_board()
    assert isinstance(ev, int)
    # best move at depth 3 should at least return a Move
    best = engine.find_best_move(max_depth=3, color=Color.WHITE)
    assert best is not None
    assert best.is_valid()

if __name__ == "__main__":
    pytest.main()