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

    def load_fen(self, fen: str):
        """Load a position from FEN (only piece placement is supported)."""

        # Clear all bitboards
        self.white_pawns = self.white_knights = self.white_bishops = 0
        self.white_rooks = self.white_queens = self.white_kings = 0
        self.black_pawns = self.black_knights = self.black_bishops = 0
        self.black_rooks = self.black_queens = self.black_kings = 0

        # Map FEN symbols to bitboards
        piece_map = {
            'P': 'white_pawns', 'N': 'white_knights', 'B': 'white_bishops',
            'R': 'white_rooks', 'Q': 'white_queens', 'K': 'white_kings',
            'p': 'black_pawns', 'n': 'black_knights', 'b': 'black_bishops',
            'r': 'black_rooks', 'q': 'black_queens', 'k': 'black_kings',
        }

        # Only parse piece placement (before first space)
        rows = fen.split()[0].split('/')
        for rank_index, row in enumerate(rows):
            file_index = 0
            for ch in row:
                if ch.isdigit():
                    file_index += int(ch)
                else:
                    square = (7 - rank_index) * 8 + file_index
                    bb_name = piece_map[ch]
                    setattr(self, bb_name, getattr(self, bb_name) | (1 << square))
                    file_index += 1

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
        Apply a pseudo‑legal move by updating the appropriate bitboards.
        Does not handle castling or en passant.
        """
        if not move.is_valid():
            return False

        start, end = move.start, move.end
        my_prefix = 'white_' if color == Color.WHITE else 'black_'
        op_prefix = 'black_' if color == Color.WHITE else 'white_'

        # 1) Remove any captured piece
        for piece in ['pawns', 'knights', 'bishops', 'rooks', 'queens', 'kings']:
            bb_attr = op_prefix + piece
            if getattr(self, bb_attr) & (1 << end):
                setattr(self, bb_attr, getattr(self, bb_attr) & ~(1 << end))
                move.move_type = MoveType.CAPTURE
                break

        # 2) Handle promotions
        if move.move_type == MoveType.PROMOTION and move.promotion_piece:
            # Clear pawn from start
            self._clear_bitboard(my_prefix + 'pawns', start)
            # Set promoted piece
            self._set_bitboard(my_prefix + move.promotion_piece.lower() + 's', end)
            return True

        # 3) Move the piece that lives on start
        for piece in ['pawns', 'knights', 'bishops', 'rooks', 'queens', 'kings']:
            bb_attr = my_prefix + piece
            if getattr(self, bb_attr) & (1 << start):
                # clear start, set end
                self._clear_bitboard(bb_attr, start)
                self._set_bitboard(bb_attr, end)
                return True

        return False

    def _clear_bitboard(self, bb_attr: str, pos: int):
        setattr(self, bb_attr, getattr(self, bb_attr) & ~(1 << pos))

    def _set_bitboard(self, bb_attr: str, pos: int):
        setattr(self, bb_attr, getattr(self, bb_attr) | (1 << pos))

    def generate_legal_moves(self, color: Color) -> List[Move]:
        """
        Create pseudo-legal moves for every piece type:
        pawns (push, double, caps, promotions), knights, bishops,
        rooks, queens, and kings.
        (No en passant or castling yet.)
        """
        moves: List[Move] = []

        # Build occupancy masks using bitwise OR
        white_occ = (
                self.white_pawns | self.white_knights | self.white_bishops |
                self.white_rooks | self.white_queens | self.white_kings
        )
        black_occ = (
                self.black_pawns | self.black_knights | self.black_bishops |
                self.black_rooks | self.black_queens | self.black_kings
        )
        all_occ = white_occ | black_occ
        own_occ = white_occ if color == Color.WHITE else black_occ
        opp_occ = black_occ if color == Color.WHITE else white_occ

        # Direction vectors
        knight_dirs = [-17, -15, -10, -6, 6, 10, 15, 17]
        king_dirs = [-9, -8, -7, -1, 1, 7, 8, 9]
        bishop_dirs = [-9, -7, 7, 9]
        rook_dirs = [-8, -1, 1, 8]

        prefix = 'white_' if color == Color.WHITE else 'black_'
        pawn_dir = 8 if color == Color.WHITE else -8
        start_rank = 1 if color == Color.WHITE else 6
        promo_rank = 7 if color == Color.WHITE else 0
        pawn_bb = getattr(self, prefix + 'pawns')

        # --- Pawn moves ---
        tmp = pawn_bb
        while tmp:
            sq = (tmp & -tmp).bit_length() - 1
            tmp &= tmp - 1

            # single push
            t1 = sq + pawn_dir
            if 0 <= t1 < 64 and not (all_occ & (1 << t1)):
                if t1 // 8 == promo_rank:
                    for promo in ['Q', 'R', 'B', 'N']:
                        moves.append(Move(sq, t1, MoveType.PROMOTION, promo))
                else:
                    moves.append(Move(sq, t1))
                # double push
                if sq // 8 == start_rank:
                    t2 = sq + 2 * pawn_dir
                    if 0 <= t2 < 64 and not (all_occ & (1 << t2)):
                        moves.append(Move(sq, t2))

            # captures
            for d in (pawn_dir - 1, pawn_dir + 1):
                tc = sq + d
                if 0 <= tc < 64 and (opp_occ & (1 << tc)):
                    if tc // 8 == promo_rank:
                        for promo in ['Q', 'R', 'B', 'N']:
                            moves.append(Move(sq, tc, MoveType.CAPTURE, promo))
                    else:
                        moves.append(Move(sq, tc, MoveType.CAPTURE))

        # --- Knight moves ---
        tmp = getattr(self, prefix + 'knights')
        while tmp:
            sq = (tmp & -tmp).bit_length() - 1
            tmp &= tmp - 1
            for d in knight_dirs:
                tgt = sq + d
                if 0 <= tgt < 64 and not (own_occ & (1 << tgt)):
                    mtype = MoveType.CAPTURE if (opp_occ & (1 << tgt)) else MoveType.NORMAL
                    moves.append(Move(sq, tgt, mtype))

        # --- Sliding pieces (bishop, rook) ---
        for dirs, name in [(bishop_dirs, 'bishops'), (rook_dirs, 'rooks')]:
            tmp = getattr(self, prefix + name)
            while tmp:
                sq = (tmp & -tmp).bit_length() - 1
                tmp &= tmp - 1
                for d in dirs:
                    t = sq
                    while True:
                        t += d
                        if not (0 <= t < 64):
                            break
                        # Prevent wrap-around
                        if abs((sq % 8) - (t % 8)) > 2 and d in (-1, 1, -9, -7, 7, 9):
                            break
                        if own_occ & (1 << t):
                            break
                        mtype = MoveType.CAPTURE if (opp_occ & (1 << t)) else MoveType.NORMAL
                        moves.append(Move(sq, t, mtype))
                        if all_occ & (1 << t):
                            break

        # --- Queen moves (bishop + rook) ---
        tmp = getattr(self, prefix + 'queens')
        while tmp:
            sq = (tmp & -tmp).bit_length() - 1
            tmp &= tmp - 1
            for d in bishop_dirs + rook_dirs:
                t = sq
                while True:
                    t += d
                    if not (0 <= t < 64):
                        break
                    if abs((sq % 8) - (t % 8)) > 2 and d in (-1, 1, -9, -7, 7, 9):
                        break
                    if own_occ & (1 << t):
                        break
                    mtype = MoveType.CAPTURE if (opp_occ & (1 << t)) else MoveType.NORMAL
                    moves.append(Move(sq, t, mtype))
                    if all_occ & (1 << t):
                        break

        # --- King moves ---
        tmp = getattr(self, prefix + 'kings')
        while tmp:
            sq = (tmp & -tmp).bit_length() - 1
            tmp &= tmp - 1
            for d in king_dirs:
                tgt = sq + d
                if 0 <= tgt < 64 and not (own_occ & (1 << tgt)):
                    mtype = MoveType.CAPTURE if (opp_occ & (1 << tgt)) else MoveType.NORMAL
                    moves.append(Move(sq, tgt, mtype))

        return moves

# ------------------------------------------------------------------------------
# Evaluator Class (with Search, Transposition Table, and Iterative Deepening)
# ------------------------------------------------------------------------------

class Evaluator:
    def __init__(self):
        self.initialize_piece_square_tables()
        self.zobrist_keys = [[random.getrandbits(64) for _ in range(64)] for _ in range(12)]
        self.zobrist_side = random.getrandbits(64)
        self.transposition_table = {}  # Maps board hash to {'depth': d, 'score': s, 'flag': flag}
        # maximum quiescence search depth
        self.max_q_depth = 2

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
        material_score = self.evaluate_material(board)
        positional_score = self.evaluate_positional(board)
        return material_score + positional_score

    def evaluate_positional(self, board: Board) -> int:
        score = 0

        piece_tables = [
            (board.white_pawns, self.white_pawn_table, 1),
            (board.white_knights, self.white_knight_table, 1),
            (board.white_bishops, self.white_bishop_table, 1),
            (board.white_rooks, self.white_rook_table, 1),
            (board.white_queens, self.white_queen_table, 1),
            (board.white_kings, self.white_king_table_mg, 1),
            (board.black_pawns, self.black_pawn_table, -1),
            (board.black_knights, self.black_knight_table, -1),
            (board.black_bishops, self.black_bishop_table, -1),
            (board.black_rooks, self.black_rook_table, -1),
            (board.black_queens, self.black_queen_table, -1),
            (board.black_kings, self.black_king_table_mg, -1),
        ]

        for bitboard, table, sign in piece_tables:
            temp_board = bitboard
            while temp_board:
                square = (temp_board & -temp_board).bit_length() - 1
                score += sign * table[square]
                temp_board &= temp_board - 1

        return score

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

    def _get_piece_char(self, board: Board, square: int) -> Optional[str]:
        # Return the single-character piece on 'square', or None.
        mapping = [
            ('P', board.white_pawns),
            ('N', board.white_knights),
            ('B', board.white_bishops),
            ('R', board.white_rooks),
            ('Q', board.white_queens),
            ('K', board.white_kings),
            ('p', board.black_pawns),
            ('n', board.black_knights),
            ('b', board.black_bishops),
            ('r', board.black_rooks),
            ('q', board.black_queens),
            ('k', board.black_kings),
        ]
        for char, bb in mapping:
            if bb & (1 << square):
                return char
        return None

    def _piece_value(self, piece: Optional[str]) -> int:
        values = {'P':100,'N':320,'B':330,'R':500,'Q':900,'K':20000,
                  'p':100,'n':320,'b':330,'r':500,'q':900,'k':20000}
        return values.get(piece, 0)

    def _order_moves(self, moves: List[Move], board: Board, maximizing_player: bool) -> List[Move]:
        # MVV-LVA move ordering: capture moves first, sorted by victim-attacker value
        scored = []
        for m in moves:
            score = 0
            if m.is_capture():
                victim = self._get_piece_char(board, m.end)
                attacker = self._get_piece_char(board, m.start)
                score = self._piece_value(victim) - self._piece_value(attacker)
            scored.append((score, m))
        scored.sort(key=lambda x: x[0], reverse=True)
        return [m for _, m in scored]

    # --------------------------------------------------------------------------
    # Quiescence Search
    # --------------------------------------------------------------------------
    def quiescence_search(self, board: Board, alpha: int, beta: int, maximizing_player: bool, qs_depth: int = 0) -> int:
        # limit quiescence recursion
        if qs_depth >= self.max_q_depth:
            return self.evaluate(board)
        stand_pat = self.evaluate(board)
        if maximizing_player:
            if stand_pat >= beta:
                return beta
            if alpha < stand_pat:
                alpha = stand_pat
            for move in board.generate_legal_moves(Color.WHITE):
                if not move.is_capture():
                    continue
                victim = self._get_piece_char(board, move.end)
                if victim in ('K', 'k'):
                    continue
                new_board = self._copy_board(board)
                if not new_board.make_move(move, Color.WHITE):
                    continue
                score = -self.quiescence_search(new_board, -beta, -alpha, False, qs_depth + 1)
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
                victim = self._get_piece_char(board, move.end)
                if victim in ('K', 'k'):
                    continue
                new_board = self._copy_board(board)
                if not new_board.make_move(move, Color.BLACK):
                    continue
                score = -self.quiescence_search(new_board, -beta, -alpha, True, qs_depth + 1)
                if score <= alpha:
                    return alpha
                if score < beta:
                    beta = score
            return beta

    # --------------------------------------------------------------------------
    # Alpha-Beta Search with Transposition Table
    # --------------------------------------------------------------------------
    # In class Evaluator:

    def search(self, board: Board, depth: int, alpha: int, beta: int, maximizing_player: bool) -> tuple[int, Optional[Move]]:
        alpha_orig = alpha
        best_move: Optional[Move] = None
        board_hash = self.generate_zobrist_hash(board, maximizing_player)

        # Transposition table lookup
        if board_hash in self.transposition_table:
            entry = self.transposition_table[board_hash]
            if entry['depth'] >= depth:
                if entry['flag'] == EXACT:
                    return entry['score'], None
                if entry['flag'] == ALPHA_FLAG and entry['score'] <= alpha:
                    return alpha, None
                if entry['flag'] == BETA_FLAG and entry['score'] >= beta:
                    return beta, None

        # Leaf node: switch to quiescence search
        if depth == 0:
            return self.quiescence_search(board, alpha, beta, maximizing_player, 0), None

        moves = board.generate_legal_moves(Color.WHITE if maximizing_player else Color.BLACK)
        # Order moves to improve alpha-beta pruning
        moves = self._order_moves(moves, board, maximizing_player)
        if not moves:
            # Checkmate or stalemate
            return (-100000, None) if maximizing_player else (100000, None)

        best_score = -math.inf if maximizing_player else math.inf
        for move in moves:
            new_board = self._copy_board(board)
            if not new_board.make_move(move, Color.WHITE if maximizing_player else Color.BLACK):
                continue
            score, _ = self.search(new_board, depth - 1, -beta, -alpha, not maximizing_player)
            score = -score

            if maximizing_player:
                if score > best_score:
                    best_score = score
                    best_move = move
                if score > alpha:
                    alpha = score
            else:
                if score < best_score:
                    best_score = score
                    best_move = move
                if score < beta:
                    beta = score

            if alpha >= beta:
                break

        # Store result in transposition table
        if best_score <= alpha_orig:
            flag = BETA_FLAG
        elif best_score >= beta:
            flag = ALPHA_FLAG
        else:
            flag = EXACT

        self.transposition_table[board_hash] = {
            'depth': depth,
            'score': best_score,
            'flag': flag
        }
        return best_score, best_move


    def iterative_deepening(self, board: Board, max_depth: int, maximizing_player: bool) -> tuple[int, Optional[Move]]:
        """
        Repeatedly deepen from 1 to max_depth and keep the best move found.
        """
        best_move: Optional[Move] = None
        best_score = 0
        for d in range(1, max_depth + 1):
            score, move = self.search(board, d, -math.inf, math.inf, maximizing_player)
            if move:
                best_move = move
            print(f"Depth {d} completed: Best score {score}, Best move: {best_move.get_move_str() if best_move else 'None'}")
            best_score = score
        return best_score, best_move

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

    def find_best_move(self, max_depth: int, color: Color) -> Optional[Move]:
        maximizing = (color == Color.WHITE)
        _, best_move = self.evaluator.iterative_deepening(self.board, max_depth, maximizing)
        return best_move
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
        best_move = engine.find_best_move(max_depth=3, color=Color.WHITE)
        print("Best move found:", best_move.get_move_str() if best_move else "None")
    else:
        print("Move was invalid.")
