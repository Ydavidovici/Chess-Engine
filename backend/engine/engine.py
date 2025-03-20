# engine/chess_engine.py

from dataclasses import dataclass
from enum import Enum, auto
from typing import List, Optional
import random
import math
import sys

# ------------------------------------------------------------------------------
# Enums and Constants
# ------------------------------------------------------------------------------

class Color(Enum):
    WHITE = auto()
    BLACK = auto()

class MoveType(Enum):
    NORMAL = auto()
    CAPTURE = auto()
    CASTLE_KINGSIDE = auto()
    CASTLE_QUEENSIDE = auto()
    PROMOTION = auto()
    EN_PASSANT = auto()
    INVALID = auto()

# Transposition table flag constants:
EXACT = 0
ALPHA_FLAG = 1
BETA_FLAG = 2

# ------------------------------------------------------------------------------
# Move Class
# ------------------------------------------------------------------------------

@dataclass
class Move:
    start: int      # 0 to 63 index
    end: int        # 0 to 63 index
    move_type: MoveType = MoveType.NORMAL
    promotion_piece: Optional[str] = None  # 'Q', 'R', 'B', 'N'

    def is_valid(self) -> bool:
        return self._in_bounds(self.start) and self._in_bounds(self.end) and self.move_type != MoveType.INVALID

    def _in_bounds(self, pos: int) -> bool:
        return 0 <= pos < 64

    def is_capture(self) -> bool:
        return self.move_type in (MoveType.CAPTURE, MoveType.EN_PASSANT)

    def get_move_str(self) -> str:
        # Converts the move to a simple algebraic notation like "e2e4", plus promotion if any.
        def pos_to_str(pos):
            file = pos % 8
            rank = pos // 8
            return chr(file + ord('a')) + str(rank + 1)
        s = pos_to_str(self.start) + pos_to_str(self.end)
        if self.promotion_piece:
            s += self.promotion_piece.upper()
        return s

# ------------------------------------------------------------------------------
# Board Class
# ------------------------------------------------------------------------------

class Board:
    def __init__(self):
        self.initialize_board()

    def initialize_board(self):
        # Bitboards represented as Python integers.
        self.white_pawns   = 0x000000000000FF00
        self.white_knights = 0x0000000000000042
        self.white_bishops = 0x0000000000000024
        self.white_rooks   = 0x0000000000000081
        self.white_queens  = 0x0000000000000008
        self.white_kings   = 0x0000000000000010

        self.black_pawns   = 0x00FF000000000000
        self.black_knights = 0x4200000000000000
        self.black_bishops = 0x2400000000000000
        self.black_rooks   = 0x8100000000000000
        self.black_queens  = 0x0800000000000000
        self.black_kings   = 0x1000000000000000

    def get_board_state(self) -> str:
        """Return a human-readable string representation of the board."""
        state = ""
        for rank in range(7, -1, -1):
            for file in range(8):
                pos = self.get_position(rank, file)
                piece = "."
                if self._is_bit_set(self.white_pawns, pos):   piece = "P"
                elif self._is_bit_set(self.white_knights, pos): piece = "N"
                elif self._is_bit_set(self.white_bishops, pos): piece = "B"
                elif self._is_bit_set(self.white_rooks, pos):   piece = "R"
                elif self._is_bit_set(self.white_queens, pos):  piece = "Q"
                elif self._is_bit_set(self.white_kings, pos):   piece = "K"
                elif self._is_bit_set(self.black_pawns, pos):   piece = "p"
                elif self._is_bit_set(self.black_knights, pos): piece = "n"
                elif self._is_bit_set(self.black_bishops, pos): piece = "b"
                elif self._is_bit_set(self.black_rooks, pos):   piece = "r"
                elif self._is_bit_set(self.black_queens, pos):  piece = "q"
                elif self._is_bit_set(self.black_kings, pos):   piece = "k"
                state += piece + " "
            state += "\n"
        return state

    def _is_bit_set(self, bitboard: int, pos: int) -> bool:
        return (bitboard & (1 << pos)) != 0

    def _set_bit(self, bitboard: int, pos: int) -> int:
        return bitboard | (1 << pos)

    def _clear_bit(self, bitboard: int, pos: int) -> int:
        return bitboard & ~(1 << pos)

    def get_position(self, rank: int, file: int) -> int:
        return rank * 8 + file

    def make_move(self, move: Move, color: Color) -> bool:
        """
        Makes a move on the board.
        Currently supports pawn moves and a basic promotion (auto-promoting to queen).
        Extend this method for full move support.
        """
        if not move.is_valid():
            return False

        start = move.start
        end = move.end
        mtype = move.move_type

        if color == Color.WHITE:
            if self._is_bit_set(self.white_pawns, start):
                self.white_pawns = self._clear_bit(self.white_pawns, start)
                self.white_pawns = self._set_bit(self.white_pawns, end)
                if mtype == MoveType.PROMOTION and move.promotion_piece:
                    self.white_pawns = self._clear_bit(self.white_pawns, end)
                    self.white_queens = self._set_bit(self.white_queens, end)
                return True
            # (Extend for other white pieces)
        else:  # Color.BLACK
            if self._is_bit_set(self.black_pawns, start):
                self.black_pawns = self._clear_bit(self.black_pawns, start)
                self.black_pawns = self._set_bit(self.black_pawns, end)
                if mtype == MoveType.PROMOTION and move.promotion_piece:
                    self.black_pawns = self._clear_bit(self.black_pawns, end)
                    self.black_queens = self._set_bit(self.black_queens, end)
                return True
            # (Extend for other black pieces)
        return False

    def generate_legal_moves(self, color: Color) -> List[Move]:
        """
        Generates legal moves for the given color.
        (For simplicity, currently only single forward pawn moves are generated.)
        """
        moves = []
        if color == Color.WHITE:
            bitboard = self.white_pawns
            while bitboard:
                square = (bitboard & -bitboard).bit_length() - 1
                bitboard &= bitboard - 1
                target = square + 8
                moves.append(Move(start=square, end=target))
        else:
            bitboard = self.black_pawns
            while bitboard:
                square = (bitboard & -bitboard).bit_length() - 1
                bitboard &= bitboard - 1
                target = square - 8
                moves.append(Move(start=square, end=target))
        return moves

    def get_piece_count(self) -> int:
        """Returns the total count of pieces on the board."""
        count = 0
        count += bin(self.white_pawns).count("1")
        count += bin(self.white_knights).count("1")
        count += bin(self.white_bishops).count("1")
        count += bin(self.white_rooks).count("1")
        count += bin(self.white_queens).count("1")
        count += bin(self.white_kings).count("1")
        count += bin(self.black_pawns).count("1")
        count += bin(self.black_knights).count("1")
        count += bin(self.black_bishops).count("1")
        count += bin(self.black_rooks).count("1")
        count += bin(self.black_queens).count("1")
        count += bin(self.black_kings).count("1")
        return count

# ------------------------------------------------------------------------------
# Evaluator Class (with Search, Transposition Table, and Iterative Deepening)
# ------------------------------------------------------------------------------

class Evaluator:
    def __init__(self):
        self.initialize_piece_square_tables()
        self.zobrist_keys = [[random.getrandbits(64) for _ in range(64)] for _ in range(12)]
        self.zobrist_side = random.getrandbits(64)
        self.transposition_table = {}  # Maps board hash to {'depth': d, 'score': s, 'flag': flag}

    def initialize_piece_square_tables(self):
        # White Piece-Square Tables
        self.white_pawn_table = [
            0, 0, 0, 0, 0, 0, 0, 0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 10, 20, 30, 30, 20, 10, 10,
            5, 5, 10, 25, 25, 10, 5, 5,
            0, 0, 0, 20, 20, 0, 0, 0,
            5, -5, -10, 0, 0, -10, -5, 5,
            5, 10, 10, -20, -20, 10, 10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
        ]

        self.white_knight_table = [
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20, 0, 0, 0, 0, -20, -40,
            -30, 0, 10, 15, 15, 10, 0, -30,
            -30, 5, 15, 20, 20, 15, 5, -30,
            -30, 0, 15, 20, 20, 15, 0, -30,
            -30, 5, 10, 15, 15, 10, 5, -30,
            -40, -20, 0, 5, 5, 0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        ]

        self.white_bishop_table = [
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 10, 10, 5, 0, -10,
            -10, 5, 5, 10, 10, 5, 5, -10,
            -10, 0, 10, 10, 10, 10, 0, -10,
            -10, 10, 10, 10, 10, 10, 10, -10,
            -10, 5, 0, 0, 0, 0, 5, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
        ]

        self.white_rook_table = [
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, 10, 10, 10, 10, 5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            0, 0, 0, 5, 5, 0, 0, 0
        ]

        self.white_queen_table = [
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -5, 0, 5, 5, 5, 5, 0, -5,
            0, 0, 5, 5, 5, 5, 0, -5,
            -10, 5, 5, 5, 5, 5, 0, -10,
            -10, 0, 5, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20
        ]

        self.white_king_table_mg = [
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            20, 20, 0, 0, 0, 0, 20, 20,
            20, 30, 10, 0, 0, 10, 30, 20
        ]

        self.white_king_table_eg = [
            -50, -40, -30, -20, -20, -30, -40, -50,
            -30, -20, -10, 0, 0, -10, -20, -30,
            -30, -10, 20, 30, 30, 20, -10, -30,
            -30, -10, 30, 40, 40, 30, -10, -30,
            -30, -10, 30, 40, 40, 30, -10, -30,
            -30, -10, 20, 30, 30, 20, -10, -30,
            -30, -30, 0, 0, 0, 0, -30, -30,
            -50, -30, -30, -30, -30, -30, -30, -50
        ]

        # Black Piece-Square Tables (mirrored vertically from White tables)
        self.black_pawn_table = self.white_pawn_table[::-1]
        self.black_knight_table = self.white_knight_table[::-1]
        self.black_bishop_table = self.white_bishop_table[::-1]
        self.black_rook_table = self.white_rook_table[::-1]
        self.black_queen_table = self.white_queen_table[::-1]
        self.black_king_table_mg = self.white_king_table_mg[::-1]
        self.black_king_table_eg = self.white_king_table_eg[::-1]


    def generate_zobrist_hash(self, board: Board, side_to_move: bool) -> int:
        hash_value = 0
        for square in range(64):
            if board._is_bit_set(board.white_pawns, square):
                hash_value ^= self.zobrist_keys[0][square]
            if board._is_bit_set(board.white_knights, square):
                hash_value ^= self.zobrist_keys[1][square]
            if board._is_bit_set(board.white_bishops, square):
                hash_value ^= self.zobrist_keys[2][square]
            if board._is_bit_set(board.white_rooks, square):
                hash_value ^= self.zobrist_keys[3][square]
            if board._is_bit_set(board.white_queens, square):
                hash_value ^= self.zobrist_keys[4][square]
            if board._is_bit_set(board.white_kings, square):
                hash_value ^= self.zobrist_keys[5][square]
        for square in range(64):
            if board._is_bit_set(board.black_pawns, square):
                hash_value ^= self.zobrist_keys[6][square]
            if board._is_bit_set(board.black_knights, square):
                hash_value ^= self.zobrist_keys[7][square]
            if board._is_bit_set(board.black_bishops, square):
                hash_value ^= self.zobrist_keys[8][square]
            if board._is_bit_set(board.black_rooks, square):
                hash_value ^= self.zobrist_keys[9][square]
            if board._is_bit_set(board.black_queens, square):
                hash_value ^= self.zobrist_keys[10][square]
            if board._is_bit_set(board.black_kings, square):
                hash_value ^= self.zobrist_keys[11][square]
        if side_to_move:
            hash_value ^= self.zobrist_side
        return hash_value

    def evaluate_material(self, board: Board) -> int:
        material = 0
        material += 100 * bin(board.white_pawns).count("1")
        material += 320 * bin(board.white_knights).count("1")
        material += 330 * bin(board.white_bishops).count("1")
        material += 500 * bin(board.white_rooks).count("1")
        material += 900 * bin(board.white_queens).count("1")
        material += 20000 * bin(board.white_kings).count("1")
        material -= 100 * bin(board.black_pawns).count("1")
        material -= 320 * bin(board.black_knights).count("1")
        material -= 330 * bin(board.black_bishops).count("1")
        material -= 500 * bin(board.black_rooks).count("1")
        material -= 900 * bin(board.black_queens).count("1")
        material -= 20000 * bin(board.black_kings).count("1")
        return material

    def evaluate(self, board: Board) -> int:
        # For now, evaluation is based solely on material.
        return self.evaluate_material(board)

    def _copy_board(self, board: Board) -> Board:
        new_board = Board()
        new_board.white_pawns = board.white_pawns
        new_board.white_knights = board.white_knights
        new_board.white_bishops = board.white_bishops
        new_board.white_rooks = board.white_rooks
        new_board.white_queens = board.white_queens
        new_board.white_kings = board.white_kings
        new_board.black_pawns = board.black_pawns
        new_board.black_knights = board.black_knights
        new_board.black_bishops = board.black_bishops
        new_board.black_rooks = board.black_rooks
        new_board.black_queens = board.black_queens
        new_board.black_kings = board.black_kings
        return new_board

    # --------------------------------------------------------------------------
    # Quiescence Search
    # --------------------------------------------------------------------------
    def quiescence_search(self, board: Board, alpha: int, beta: int, maximizing_player: bool) -> int:
        stand_pat = self.evaluate(board)
        if maximizing_player:
            if stand_pat >= beta:
                return beta
            if alpha < stand_pat:
                alpha = stand_pat
            for move in board.generate_legal_moves(Color.WHITE):
                if not move.is_capture():
                    continue
                new_board = self._copy_board(board)
                if not new_board.make_move(move, Color.WHITE):
                    continue
                score = -self.quiescence_search(new_board, -beta, -alpha, False)
                if score >= beta:
                    return beta
                if score > alpha:
                    alpha = score
            return alpha
        else:
            if stand_pat <= alpha:
                return alpha
            if beta > stand_pat:
                beta = stand_pat
            for move in board.generate_legal_moves(Color.BLACK):
                if not move.is_capture():
                    continue
                new_board = self._copy_board(board)
                if not new_board.make_move(move, Color.BLACK):
                    continue
                score = -self.quiescence_search(new_board, -beta, -alpha, True)
                if score <= alpha:
                    return alpha
                if score < beta:
                    beta = score
            return beta

    # --------------------------------------------------------------------------
    # Alpha-Beta Search with Transposition Table
    # --------------------------------------------------------------------------
    def search(self, board: Board, depth: int, alpha: int, beta: int, maximizing_player: bool) -> int:
        alpha_orig = alpha
        board_hash = self.generate_zobrist_hash(board, maximizing_player)
        if board_hash in self.transposition_table:
            entry = self.transposition_table[board_hash]
            if entry['depth'] >= depth:
                if entry['flag'] == EXACT:
                    return entry['score']
                elif entry['flag'] == ALPHA_FLAG and entry['score'] <= alpha:
                    return alpha
                elif entry['flag'] == BETA_FLAG and entry['score'] >= beta:
                    return beta
        if depth == 0:
            return self.quiescence_search(board, alpha, beta, maximizing_player)
        moves = board.generate_legal_moves(Color.WHITE if maximizing_player else Color.BLACK)
        if not moves:
            # Terminal condition: assume checkmate/stalemate score.
            return -100000 if maximizing_player else 100000
        best_score = -math.inf if maximizing_player else math.inf
        for move in moves:
            new_board = self._copy_board(board)
            if not new_board.make_move(move, Color.WHITE if maximizing_player else Color.BLACK):
                continue
            score = -self.search(new_board, depth - 1, -beta, -alpha, not maximizing_player)
            if maximizing_player:
                if score > best_score:
                    best_score = score
                if score > alpha:
                    alpha = score
            else:
                if score < best_score:
                    best_score = score
                if score < beta:
                    beta = score
            if alpha >= beta:
                break
        # Determine transposition table flag.
        if best_score <= alpha_orig:
            flag = BETA_FLAG
        elif best_score >= beta:
            flag = ALPHA_FLAG
        else:
            flag = EXACT
        self.transposition_table[board_hash] = {'depth': depth, 'score': best_score, 'flag': flag}
        return best_score

    def iterative_deepening(self, board: Board, max_depth: int, maximizing_player: bool) -> int:
        best_score = 0
        for depth in range(1, max_depth + 1):
            best_score = self.search(board, depth, -math.inf, math.inf, maximizing_player)
            print(f"Depth {depth} completed with score: {best_score}")
        return best_score

# ------------------------------------------------------------------------------
# Engine Class
# ------------------------------------------------------------------------------

class Engine:
    def __init__(self):
        self.board = Board()
        self.evaluator = Evaluator()

    def initialize(self):
        self.board.initialize_board()

    def new_game(self) -> str:
        self.initialize()
        return self.board.get_board_state()

    def get_board_state(self) -> str:
        return self.board.get_board_state()

    def make_move(self, move: Move, color: Color) -> bool:
        if not self.board.make_move(move, color):
            print(f"Invalid move: {move.get_move_str()}", file=sys.stderr)
            return False
        return True

    def evaluate_board(self) -> int:
        return self.evaluator.evaluate(self.board)

    def search_best_move(self, max_depth: int, color: Color) -> int:
        """
        Uses iterative deepening to search for the best move evaluation.
        (In a full implementation, you would extract the actual move.)
        """
        maximizing = True if color == Color.WHITE else False
        return self.evaluator.iterative_deepening(self.board, max_depth, maximizing)

# ------------------------------------------------------------------------------
# Example Usage (for testing)
# ------------------------------------------------------------------------------
if __name__ == "__main__":
    engine = Engine()
    print("Initial Board:")
    print(engine.get_board_state())

    # Example move: white pawn from e2 to e4 (positions 12 -> 28)
    move = Move(start=12, end=28, move_type=MoveType.NORMAL)
    if engine.make_move(move, Color.WHITE):
        print("After move e2e4:")
        print(engine.get_board_state())
        print("Board evaluation:", engine.evaluate_board())
        print("Searching best move for white (up to depth 3):")
        best_score = engine.search_best_move(max_depth=3, color=Color.WHITE)
        print("Best evaluation found:", best_score)
    else:
        print("Move was invalid.")
