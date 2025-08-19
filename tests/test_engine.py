# test_chessengine.py
import chessengine as ce

def test_board():
    print("** BOARD API **")
    print("chessengine module path:", getattr(ce, "__file__", "<unknown>"))

    b = ce.Board()
    # Board() usually initializes; this next line is fine but optional.
    b.initialize()

    # --- Startpos sanity ---
    start_fen = b.to_fen()
    print("Start FEN:", start_fen)
    start_legal = b.generate_legal_moves()
    # expect exactly 20 legal moves from the start position
    assert len(start_legal) == 20, f"expected 20 start moves, got {len(start_legal)}"

    # --- Scholar position ---
    scholar = "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 2 3"
    b.load_fen(scholar)
    print("Loaded FEN:", b.to_fen())

    # Legal moves as Move objects
    legals = b.generate_legal_moves()
    leg_str = [m.toString() for m in legals]
    print(f"{len(legals)} legal moves from this position:", leg_str[:8], "...")

    # Focus on moves from a7 – only a7a6/a7a5 should exist (no wrap like a7h5)
    a7_moves = sorted(s for s in leg_str if s.startswith("a7"))
    print("Moves from a7:", a7_moves)
    assert a7_moves == ["a7a5", "a7a6"], f"unexpected a7 moves: {a7_moves}"
    assert "a7h5" not in leg_str

    # Make a move & undo
    move = a7_moves[0]
    print("Applying move:", move)
    ok = b.make_move(move)
    assert ok, f"make_move({move}) failed"
    print("FEN after move:", b.to_fen())
    print("Undoing move")
    b.unmake_move()
    print("Back to FEN:", b.to_fen())
    assert b.to_fen() == scholar, "FEN mismatch after unmake_move()"

    # Attacked-square probe: check if e4 (index 4) is attacked by Black in the scholar FEN
    # (In this FEN, it's Black to move; '1' here means BLACK in your API)
    is_attacked_by_black = b.is_square_attacked(4, ce.Color.BLACK)
    print(f"Is e4 (idx 4) attacked by BLACK? {is_attacked_by_black}")

def test_engine():
    print("\n** ENGINE API **")
    e = ce.Engine()
    e.reset()
    initial_fen = e.get_fen()
    print("Engine start FEN:", initial_fen)

    scholar = "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 2 3"
    ok = e.set_position(scholar)
    print("Set position succeeded?", ok)
    print("Engine FEN is now:", e.get_fen())
    assert ok and e.get_fen() == scholar

    leg = e.legal_moves()
    print("Engine sees", len(leg), "legal moves:", leg[:8], "...")
    # assert same a7-only behavior via engine API
    a7 = sorted([m for m in leg if m.startswith("a7")])
    print("Engine moves from a7:", a7)
    assert a7 == ["a7a5", "a7a6"]
    assert "a7h5" not in leg

    # apply a legal move & undo
    move = a7[0]
    print("Engine applying move:", move, "->", e.apply_move(move))
    print("FEN after engine move:", e.get_fen())
    print("Undoing engine move:", e.undo_move())
    print("Back to FEN:", e.get_fen())
    assert e.get_fen() == scholar

    # evaluation
    score = e.evaluate()
    print("Static eval of starting position:", score)

    # quick playout
    settings = ce.PlaySettings()
    settings.depth = 3
    settings.tt_size_mb = 16
    settings.time_left_ms = 1000
    settings.increment_ms = 0
    settings.moves_to_go = 30

    print("\n-- Let engine play a few moves --")
    e.reset()
    moves = []
    for ply in range(6):
        if e.is_game_over():
            print("Game over at ply", ply)
            break
        mv = e.play_move(settings)
        moves.append(mv)
        print(f"Engine ply {ply+1}:", mv)

    data = e.get_game_data()
    print("Game moves:", data.moves)

    if hasattr(e, "last_search_info"):
        info = e.last_search_info()
        print("Last search info:", dict(
            nodes=info.nodes,
            tt_hits=info.tt_hits,
            depth=info.depth,
            time_ms=info.time_ms
        ))

if __name__ == "__main__":
    test_board()
    test_engine()
