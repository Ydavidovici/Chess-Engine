#!/usr/bin/env python3
import argparse
import sys
from pyengine import Engine, Move, Color, MoveType

def square_to_index(sq: str) -> int:
    file = ord(sq[0]) - ord('a')
    rank = int(sq[1]) - 1
    return rank * 8 + file


def main():
    parser = argparse.ArgumentParser(prog="chess-engine")
    subparsers = parser.add_subparsers(dest="cmd", required=True)

    subparsers.add_parser("new", help="Start a new game")
    subparsers.add_parser("state", help="Print current board")
    move_parser = subparsers.add_parser(
        "move", help="Make a move (e.g. e2e4 or e7e8Q)"
    )
    move_parser.add_argument("mv", help="4-5 char move string")
    subparsers.add_parser("eval", help="Evaluate current position")

    args = parser.parse_args()

    engine = Engine()

    if args.cmd == "new":
        engine.new_game()
        engine.print_board()

    elif args.cmd == "state":
        engine.print_board()

    elif args.cmd == "move":
        mv = args.mv
        try:
            start = square_to_index(mv[0:2])
            end = square_to_index(mv[2:4])
        except Exception:
            print(f"Invalid move format: {mv}", file=sys.stderr)
            sys.exit(1)

        move = Move()
        move.start = start
        move.end = end
        if len(mv) == 5:
            move.type = MoveType.PROMOTION
            move.promo = mv[4].upper()

        if engine.make_move(move, Color.WHITE):
            print(f"After {mv}:")
            engine.print_board()
        else:
            print(f"Invalid move: {mv}", file=sys.stderr)
            sys.exit(1)

    elif args.cmd == "eval":
        score = engine.evaluate_board()
        print(f"Eval: {score}")

if __name__ == "__main__":
    main()