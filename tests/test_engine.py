# test_chessengine.py
import chessengine as ce

def test_board():
    print("** BOARD API **")
    b = ce.Board()
    b.initialize()
    start_fen = b.to_fen()
    print("Start FEN:", start_fen)

    # Load a custom position (e.g. Scholar's Mate after 1.e4 e5 2.Qh5 Nc6 3.Bc4 Nf6)
    scholar = "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 2 3"
    b.load_fen(scholar)
    print("Loaded FEN:", b.to_fen())

    # Legal moves
    legals = b.generate_legal_moves()
    print(f"{len(legals)} legal moves from this position:", [m.toString() for m in legals[:8]], "...")

    # Make a move & undo
    move = legals[0].toString()
    print("Applying move:", move)
    b.make_move(move)
    print("FEN after move:", b.to_fen())
    print("Undoing move")
    b.unmake_move()
    print("Back to FEN:", b.to_fen())

    # Inspectors
    stm = b.side_to_move()
    print("Side to move (0=WHITE,1=BLACK):", int(stm))
    sq = 4  # e4 square index
    attacked = b.is_square_attacked(sq, stm)
    print(f"Is square {sq} attacked by side {int(stm)}?", attacked)

def test_engine():
    print("\n** ENGINE API **")
    e = ce.Engine()
    # reset and FEN roundtrip
    e.reset()
    initial_fen = e.get_fen()
    print("Engine start FEN:", initial_fen)

    # set position
    scholar = "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 2 3"
    ok = e.set_position(scholar)
    print("Set position succeeded?", ok)
    print("Engine FEN is now:", e.get_fen())

    # apply a legal move
    leg = e.legal_moves()
    print("Engine sees", len(leg), "legal moves:", leg[:8], "...")
    move = leg[0]
    print("Engine applying move:", move, "->", e.apply_move(move))
    print("FEN after engine move:", e.get_fen())

    # undo
    print("Undoing engine move:", e.undo_move())
    print("Back to FEN:", e.get_fen())

    # evaluation
    score = e.evaluate()
    print("Static eval of starting position:", score)

    # play a playout
    settings = ce.PlaySettings()
    settings.depth = 3
    settings.tt_size_mb = 16
    settings.time_left_ms = 0       # use depth-only search
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

    # If you still have SearchInfo bound:
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