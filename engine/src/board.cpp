// engine/src/board.cpp
#include "board.h"
#include <sstream>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <random>

//=== Zobrist tables ===
static uint64_t pieceKeys[2][Board::PieceTypeCount][64];
static uint64_t sideKey;
static uint64_t castleKeys[16];
static uint64_t epFileKey[8];

static uint64_t rand64() {
    static std::mt19937_64 rng{ std::random_device{}() };
    return rng();
}

// One-time Zobrist initializer
static struct ZobristInit {
    ZobristInit() {
        for (int c = 0; c < 2; ++c)
            for (int p = 0; p < Board::PieceTypeCount; ++p)
                for (int sq = 0; sq < 64; ++sq)
                    pieceKeys[c][p][sq] = rand64();
        sideKey = rand64();
        for (int i = 0; i < 16; ++i)   castleKeys[i] = rand64();
        for (int f = 0; f < 8; ++f)     epFileKey[f]  = rand64();
    }
} _zinit;

static std::string stripClocks(const std::string& fen) {
    int spaces = 0, i = 0;
    for (; i < (int)fen.size() && spaces < 4; ++i)
        if (fen[i] == ' ') ++spaces;
    return fen.substr(0, i);
}

Board::Board() {
    initialize();
}

void Board::initialize() {
    // Clear all bitboards
    whiteBB.fill(0);
    blackBB.fill(0);

    // Starting position
    whiteBB[PAWN]   = 0x000000000000FF00ULL;
    whiteBB[KNIGHT] = 0x0000000000000042ULL;
    whiteBB[BISHOP] = 0x0000000000000024ULL;
    whiteBB[ROOK]   = 0x0000000000000081ULL;
    whiteBB[QUEEN]  = 0x0000000000000008ULL;
    whiteBB[KING]   = 0x0000000000000010ULL;
    blackBB[PAWN]   = 0x00FF000000000000ULL;
    blackBB[KNIGHT] = 0x4200000000000000ULL;
    blackBB[BISHOP] = 0x2400000000000000ULL;
    blackBB[ROOK]   = 0x8100000000000000ULL;
    blackBB[QUEEN]  = 0x0800000000000000ULL;
    blackBB[KING]   = 0x1000000000000000ULL;

    // Game state
    side_to_move      = Color::WHITE;
    castling_rights   = 0b1111;
    en_passant_square = -1;
    halfmove_clock    = 0;
    fullmove_number   = 1;

    history.clear();
    positionHistory.clear();

    // Compute initial Zobrist hash
    zobrist_ = 0;
    for (int c = 0; c < 2; ++c) {
        const auto &bbArr = (c==0 ? whiteBB : blackBB);
        for (int p = 0; p < PieceTypeCount; ++p) {
            uint64_t bb = bbArr[p];
            while (bb) {
                int sq = __builtin_ctzll(bb);
                bb &= bb - 1;
                zobrist_ ^= pieceKeys[c][p][sq];
            }
        }
    }
    if (side_to_move == Color::BLACK) zobrist_ ^= sideKey;
    zobrist_ ^= castleKeys[castling_rights];
    if (en_passant_square != -1)
        zobrist_ ^= epFileKey[en_passant_square % 8];

    positionHistory.push_back(stripClocks(toFEN()));
}

void Board::loadFEN(const std::string& fen) {
    // Parse FEN into board state
    whiteBB.fill(0);
    blackBB.fill(0);
    std::istringstream iss(fen);
    std::string placement, stm, cr, ep;
    iss >> placement >> stm >> cr >> ep >> halfmove_clock >> fullmove_number;

    int rank = 7, file = 0;
    for (char c : placement) {
        if (c == '/') { --rank; file = 0; continue; }
        if (std::isdigit(c)) { file += c - '0'; continue; }
        int sq = rank*8 + file++;
        switch (c) {
            case 'P': setBit(whiteBB[PAWN], sq); break;
            case 'N': setBit(whiteBB[KNIGHT], sq); break;
            case 'B': setBit(whiteBB[BISHOP], sq); break;
            case 'R': setBit(whiteBB[ROOK], sq); break;
            case 'Q': setBit(whiteBB[QUEEN], sq); break;
            case 'K': setBit(whiteBB[KING], sq); break;
            case 'p': setBit(blackBB[PAWN], sq); break;
            case 'n': setBit(blackBB[KNIGHT], sq); break;
            case 'b': setBit(blackBB[BISHOP], sq); break;
            case 'r': setBit(blackBB[ROOK], sq); break;
            case 'q': setBit(blackBB[QUEEN], sq); break;
            case 'k': setBit(blackBB[KING], sq); break;
        }
    }
    side_to_move = (stm == "w" ? Color::WHITE : Color::BLACK);
    castling_rights = 0;
    if (cr.find('K')!=std::string::npos) castling_rights |= 0b0001;
    if (cr.find('Q')!=std::string::npos) castling_rights |= 0b0010;
    if (cr.find('k')!=std::string::npos) castling_rights |= 0b0100;
    if (cr.find('q')!=std::string::npos) castling_rights |= 0b1000;
    if (ep != "-") {
        int f = ep[0] - 'a', r = ep[1] - '1';
        en_passant_square = r*8 + f;
    } else en_passant_square = -1;

    // 1) Clear any old undo/history
    history.clear();
    positionHistory.clear();

    // 2) Recompute zobrist_ from this position
    zobrist_ = 0;
    for (int c = 0; c < 2; ++c) {
        const auto &bbArr = (c == 0 ? whiteBB : blackBB);
        for (int p = 0; p < PieceTypeCount; ++p) {
            uint64_t bb = bbArr[p];
            while (bb) {
                int sq = __builtin_ctzll(bb);
                bb &= bb - 1;
                zobrist_ ^= pieceKeys[c][p][sq];
            }
        }
    }
    if (side_to_move == Color::BLACK) zobrist_ ^= sideKey;
    zobrist_ ^= castleKeys[castling_rights];
    if (en_passant_square != -1)
        zobrist_ ^= epFileKey[en_passant_square % 8];

    // 3) Record this position (minus clocks) for repetition detection
    positionHistory.push_back(stripClocks(fen));
}

std::string Board::toFEN() const {
    std::string fen;
    // Piece placement
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            char piece = 0;
            for (int i = 0; i < PieceTypeCount; ++i) {
                if (testBit(whiteBB[i], sq)) { piece = "PNBRQK"[i]; break; }
                if (testBit(blackBB[i], sq)) { piece = "pnbrqk"[i]; break; }
            }
            if (piece) {
                if (empty) { fen += char('0' + empty); empty = 0; }
                fen += piece;
            } else {
                empty++;
            }
        }
        if (empty) fen += char('0' + empty);
        if (rank) fen += '/';
    }
    // Side to move
    fen += ' ';
    fen += (side_to_move == Color::WHITE ? 'w' : 'b');
    // Castling rights
    fen += ' ';
    std::string cr;
    if (castling_rights & 0b0001) cr += 'K';
    if (castling_rights & 0b0010) cr += 'Q';
    if (castling_rights & 0b0100) cr += 'k';
    if (castling_rights & 0b1000) cr += 'q';
    fen += (cr.empty() ? "-" : cr);
    // En passant
    fen += ' ';
    if (en_passant_square != -1) {
        int f = en_passant_square % 8;
        int r = en_passant_square / 8;
        fen += char('a' + f);
        fen += char('1' + r);
    } else {
        fen += '-';
    }
    // Halfmove & fullmove clocks
    fen += ' ' + std::to_string(halfmove_clock);
    fen += ' ' + std::to_string(fullmove_number);
    return fen;
}

uint64_t Board::occupancy(Color c) const {
    uint64_t occ = 0;
    const auto& bb = (c == Color::WHITE ? whiteBB : blackBB);
    for (auto b : bb) occ |= b;
    return occ;
}

uint64_t Board::pieceBB(Color c, PieceIndex pi) const {
    return (c == Color::WHITE ? whiteBB[pi] : blackBB[pi]);
}

std::vector<Move> Board::generatePseudoMoves() const {
    std::vector<Move> moves;
    Color c = side_to_move;

    uint64_t whiteOcc = occupancy(Color::WHITE);
    uint64_t blackOcc = occupancy(Color::BLACK);
    uint64_t allOcc   = whiteOcc | blackOcc;
    uint64_t ownOcc   = (c==Color::WHITE ? whiteOcc : blackOcc);
    uint64_t oppOcc   = (c==Color::WHITE ? blackOcc : whiteOcc);

    // 1) Pawns
    uint64_t pawnBB  = (c==Color::WHITE ? whiteBB[PAWN] : blackBB[PAWN]);
    int      dir     = (c==Color::WHITE ?  8 : -8);
    int      startR  = (c==Color::WHITE ?  1 : 6);
    int      promoR  = (c==Color::WHITE ?  7 : 0);
    uint64_t tmp = pawnBB;
    while (tmp) {
        int sq = __builtin_ctzll(tmp);
        tmp &= tmp - 1;
        // single push
        int t1 = sq + dir;
        if (inBounds(t1) && !(allOcc & (1ULL<<t1))) {
            if (t1/8 == promoR) {
                for (char p: {'Q','R','B','N'})
                    moves.emplace_back(sq, t1, MoveType::PROMOTION, p);
            } else {
                moves.emplace_back(sq, t1);
                // double push
                if (sq/8 == startR) {
                    int t2 = sq + 2*dir;
                    if (inBounds(t2) && !(allOcc & (1ULL<<t2)))
                        moves.emplace_back(sq, t2);
                }
            }
        }
        // captures and en-passant
        for (int d : {dir-1, dir+1}) {
            int tc = sq + d;
            if (!inBounds(tc)) continue;
            if (oppOcc & (1ULL<<tc)) {
                // normal capture
                if (tc/8 == promoR) {
                    for (char p: {'Q','R','B','N'})
                        moves.emplace_back(sq, tc, MoveType::CAPTURE, p);
                } else {
                    moves.emplace_back(sq, tc, MoveType::CAPTURE);
                }
            }
            else if (tc == en_passant_square) {
                // en-passant capture
                moves.emplace_back(sq, tc, MoveType::EN_PASSANT);
            }
        }
    }

    // 2) Knights
    static const int knightDirs[8] = {-17,-15,-10,-6,6,10,15,17};
    tmp = (c==Color::WHITE ? whiteBB[KNIGHT] : blackBB[KNIGHT]);
    while (tmp) {
        int sq = __builtin_ctzll(tmp);
        tmp &= tmp - 1;
        for (int d : knightDirs) {
            int t = sq + d;
            if (!inBounds(t)) continue;
            if (!(ownOcc & (1ULL<<t))) {
                MoveType mt = (oppOcc & (1ULL<<t)) ? MoveType::CAPTURE
                                                   : MoveType::NORMAL;
                moves.emplace_back(sq, t, mt);
            }
        }
    }

    // 3) Sliding pieces
    auto slide = [&](uint64_t bb,
                     const int fdir[], const int rdir[], int nDir) {
        uint64_t scan = bb;
        while (scan) {
            int sq = __builtin_ctzll(scan);
            scan &= scan - 1;
            int sf = sq % 8, sr = sq / 8;
            for (int i = 0; i < nDir; ++i) {
                int f = sf, r = sr;
                while (true) {
                    f += fdir[i];  r += rdir[i];
                    if (f<0||f>7||r<0||r>7) break;
                    int t = r*8 + f;
                    MoveType mt = (oppOcc & (1ULL<<t)) ? MoveType::CAPTURE
                                                       : MoveType::NORMAL;
                    moves.emplace_back(sq, t, mt);
                    if (allOcc & (1ULL<<t)) break;
                }
            }
        }
    };
    // rook directions
    static const int rf[4]={-1,1,0,0}, rr[4]={0,0,-1,1};
    slide((c==Color::WHITE?whiteBB[ROOK]:blackBB[ROOK]), rf, rr, 4);
    // bishop directions
    static const int bf[4]={-1,1,-1,1}, br[4]={-1,-1,1,1};
    slide((c==Color::WHITE?whiteBB[BISHOP]:blackBB[BISHOP]), bf, br, 4);
    // queen = rook + bishop
    slide((c==Color::WHITE?whiteBB[QUEEN]:blackBB[QUEEN]),  rf, rr, 4);
    slide((c==Color::WHITE?whiteBB[QUEEN]:blackBB[QUEEN]),  bf, br, 4);

    // 4) King (one square)
    static const int kingDirs[8] = {-9,-8,-7,-1,1,7,8,9};
    tmp = (c==Color::WHITE ? whiteBB[KING] : blackBB[KING]);
    while (tmp) {
        int sq = __builtin_ctzll(tmp);
        tmp &= tmp - 1;
        for (int d : kingDirs) {
            int t = sq + d;
            if (!inBounds(t)) continue;
            int f0 = sq % 8, f1 = t % 8;
            if (std::abs(f1 - f0) > 1) continue;
            if (!(ownOcc & (1ULL<<t))) {
                MoveType mt = (oppOcc & (1ULL<<t)) ? MoveType::CAPTURE
                                                   : MoveType::NORMAL;
                moves.emplace_back(sq, t, mt);
            }
        }
    }

    // 5) Castling (rights & empty squares only)
    if (c==Color::WHITE) {
        if ((castling_rights&0b0001) && !(allOcc&((1ULL<<5)|(1ULL<<6))))
            moves.emplace_back(4,6,MoveType::CASTLE_KINGSIDE);
        if ((castling_rights&0b0010) && !(allOcc&((1ULL<<1)|(1ULL<<2)|(1ULL<<3))))
            moves.emplace_back(4,2,MoveType::CASTLE_QUEENSIDE);
    } else {
        if ((castling_rights&0b0100) && !(allOcc&((1ULL<<61)|(1ULL<<62))))
            moves.emplace_back(60,62,MoveType::CASTLE_KINGSIDE);
        if ((castling_rights&0b1000) && !(allOcc&((1ULL<<57)|(1ULL<<58)|(1ULL<<59))))
            moves.emplace_back(60,58,MoveType::CASTLE_QUEENSIDE);
    }

    return moves;
}

// --- new helper: findKing() ---
int Board::findKing(Color c) const {
    uint64_t kingBB = (c == Color::WHITE ? whiteBB[KING] : blackBB[KING]);
    assert(kingBB != 0);
    return __builtin_ctzll(kingBB);
}

// --- new helper: isSquareAttacked() ---
bool Board::isSquareAttacked(int sq, Color by) const {
    // occupancy and bitboards of attacker
    uint64_t occ      = occupancy(Color::WHITE) | occupancy(Color::BLACK);
    uint64_t ourOcc   = occupancy(by);
    uint64_t oppOcc   = occupancy(by == Color::WHITE ? Color::BLACK : Color::WHITE);

    // 1) pawn attacks
    if (by == Color::WHITE) {
        // white pawns attack from sq-7 and sq-9
        for (int d : { -7, -9 }) {
            int p = sq + d;
            if (inBounds(p) && (whiteBB[PAWN] & (1ULL << p))) {
                // file‐wrap: ensure pawn actually was on correct file
                int df = std::abs((p % 8) - (sq % 8));
                if (df == 1) return true;
            }
        }
    } else {
        // black pawns attack from sq+7 and sq+9
        for (int d : { +7, +9 }) {
            int p = sq + d;
            if (inBounds(p) && (blackBB[PAWN] & (1ULL << p))) {
                int df = std::abs((p % 8) - (sq % 8));
                if (df == 1) return true;
            }
        }
    }

    // 2) knight attacks
    static constexpr int knightDirs[8] = { -17,-15,-10,-6,6,10,15,17 };
    for (int d : knightDirs) {
        int p = sq + d;
        if (!inBounds(p)) continue;
        int df = std::abs((p % 8) - (sq % 8));
        if (df > 2) continue;  // wrap guard
        if ( (by == Color::WHITE ? whiteBB[KNIGHT] : blackBB[KNIGHT]) & (1ULL << p) )
            return true;
    }

    // 3) sliding: bishop/queen diagonals
    static constexpr int bishopDirs[4] = { -9, -7, 7, 9 };
    for (int d : bishopDirs) {
        int p = sq;
        while (true) {
            int f0 = p % 8;
            p += d;
            if (!inBounds(p)) break;
            int f1 = p % 8;
            if (std::abs(f1 - f0) != 1) break; // wrap
            uint64_t mask = 1ULL << p;
            if (occ & mask) {
                auto bb = by == Color::WHITE ? whiteBB[BISHOP] | whiteBB[QUEEN]
                                             : blackBB[BISHOP] | blackBB[QUEEN];
                if (bb & mask) return true;
                break;
            }
        }
    }

    // 4) sliding: rook/queen ranks & files
    static constexpr int rookDirs[4] = { -8, -1, 1, 8 };
    for (int d : rookDirs) {
        int p = sq;
        while (true) {
            int r0 = p / 8, f0 = p % 8;
            p += d;
            if (!inBounds(p)) break;
            int r1 = p / 8, f1 = p % 8;
            // wrap guard for horizontal moves
            if (d == -1 || d == +1) {
                if (r1 != r0) break;
            }
            uint64_t mask = 1ULL << p;
            if (occ & mask) {
                auto bb = by == Color::WHITE ? whiteBB[ROOK] | whiteBB[QUEEN]
                                             : blackBB[ROOK] | blackBB[QUEEN];
                if (bb & mask) return true;
                break;
            }
        }
    }

    // 5) king attacks (adjacent)
    static constexpr int kingDirs[8] = { -9,-8,-7,-1,1,7,8,9 };
    for (int d : kingDirs) {
        int p = sq + d;
        if (!inBounds(p)) continue;
        int df = std::abs((p % 8) - (sq % 8));
        if ((d == -1 || d == +1) && df != 1) continue;  // horizontal wrap
        if ( (by==Color::WHITE ? whiteBB[KING] : blackBB[KING]) & (1ULL<<p) )
            return true;
    }

    return false;
}

std::vector<Move> Board::generateLegalMoves() const {
    // First, generate all pseudo-legal moves
    auto pseudo = generatePseudoMoves();

    // If side to move has no king (e.g., pawn-only positions), skip the self-check filter
    if (!pieceBB(side_to_move, KING))
        return pseudo;

    std::vector<Move> legal;
    legal.reserve(pseudo.size());

    // Prepare a bitboard for the opponent's king to skip any king-captures
    Color them = (side_to_move == Color::WHITE ? Color::BLACK : Color::WHITE);
    uint64_t oppKingBB = pieceBB(them, KING);

    // Filter out moves that capture the opponent's king or leave you in check
    for (auto const &m : pseudo) {
        // Skip king-captures entirely
        if (oppKingBB & (1ULL << m.end))
            continue;
        Board copy = *this;
        if (copy.makeMove(m))
            legal.push_back(m);
    }

    return legal;
}

bool Board::makeMove(const Move& m) {
    // Save state
    Undo u;
    u.castling_rights    = castling_rights;
    u.en_passant_square  = en_passant_square;
    u.halfmove_clock     = halfmove_clock;
    u.fullmove_number    = fullmove_number;
    u.move               = m;
    u.is_pawn_double     = false;
    u.is_castling        = false;
    u.castling_rook_from = -1;
    u.castling_rook_to   = -1;

    uint64_t fromMask = 1ULL << m.start;
    uint64_t toMask   = 1ULL << m.end;
    Color   us       = side_to_move;
    Color   them     = (us == Color::WHITE ? Color::BLACK : Color::WHITE);

    // 1) Identify moved piece
    PieceIndex moved = PAWN;
    for (int i = 0; i < PieceTypeCount; ++i) {
        uint64_t bb = (us == Color::WHITE ? whiteBB[i] : blackBB[i]);
        if (testBit(bb, m.start)) { moved = static_cast<PieceIndex>(i); break; }
    }
    u.moved_piece = moved;

    // 2) Remove captured piece
    PieceIndex captured = PieceTypeCount;
    for (int i = 0; i < PieceTypeCount; ++i) {
        auto &bb = (them == Color::WHITE ? whiteBB[i] : blackBB[i]);
        if (testBit(bb, m.end)) {
            clearBit(bb, m.end);
            captured = static_cast<PieceIndex>(i);
            break;
        }
    }
    // En passant capture
    if (m.type == MoveType::EN_PASSANT) {
        int epSq = (us == Color::WHITE ? m.end - 8 : m.end + 8);
        auto &bb = (them == Color::WHITE ? whiteBB[PAWN] : blackBB[PAWN]);
        clearBit(bb, epSq);
        captured = PAWN;
    }
    u.captured_piece = captured;

    // 3) Castling rook move
    if (m.type == MoveType::CASTLE_KINGSIDE || m.type == MoveType::CASTLE_QUEENSIDE) {
        u.is_castling = true;
        int rf = (m.type == MoveType::CASTLE_KINGSIDE ? m.start + 3 : m.start - 4);
        int rt = (m.type == MoveType::CASTLE_KINGSIDE ? m.start + 1 : m.start - 1);
        u.castling_rook_from = rf;
        u.castling_rook_to   = rt;
        auto &rb = (us == Color::WHITE ? whiteBB[ROOK] : blackBB[ROOK]);
        clearBit(rb, rf);
        setBit(  rb, rt);
    }

    // 4) Move or promote
    auto &src = (us == Color::WHITE ? whiteBB[moved] : blackBB[moved]);
    clearBit(src, m.start);
    if (m.type == MoveType::PROMOTION && m.promo) {
        PieceIndex promo = QUEEN;
        switch (m.promo) {
            case 'R': promo = ROOK;   break;
            case 'B': promo = BISHOP; break;
            case 'N': promo = KNIGHT; break;
        }
        auto &dst = (us == Color::WHITE ? whiteBB[promo] : blackBB[promo]);
        setBit(dst, m.end);
    } else {
        setBit(src, m.end);
    }

    // 5) Update castling rights
    if (moved == KING) {
        if (us == Color::WHITE) castling_rights &= 0b1100;
        else                     castling_rights &= 0b0011;
    } else if (moved == ROOK) {
        if      (m.start == 0)  castling_rights &= 0b1110;
        else if (m.start == 7)  castling_rights &= 0b1101;
        else if (m.start == 56) castling_rights &= 0b1011;
        else if (m.start == 63) castling_rights &= 0b0111;
    }
    if (captured == ROOK) {
        if      (m.end == 0)   castling_rights &= 0b1110;
        else if (m.end == 7)   castling_rights &= 0b1101;
        else if (m.end == 56)  castling_rights &= 0b1011;
        else if (m.end == 63)  castling_rights &= 0b0111;
    }

    // 6) En passant target
    en_passant_square = -1;
    if (moved == PAWN && abs((m.end / 8) - (m.start / 8)) == 2) {
        en_passant_square = (m.start + m.end) / 2;
        u.is_pawn_double = true;
    }

    // 7) Clocks
    if (moved == PAWN || captured != PieceTypeCount) halfmove_clock = 0;
    else                                              ++halfmove_clock;
    if (us == Color::BLACK) ++fullmove_number;

    side_to_move = them;
    history.push_back(u);

    // ** if we actually have a king, reject any move that leaves it in check **
      if ( pieceBB(us, KING) == 0
        || isSquareAttacked(findKing(us), them) ) {
        // undo and fail
        unmakeMove();
        return false;
    }

    positionHistory.push_back(stripClocks(toFEN()));
    return true;
}

void Board::unmakeMove() {
    assert(!history.empty());
    Undo u = history.back();
    history.pop_back();

    Move m = u.move;
    // we just flipped side_to_move in makeMove, so swap back:
    Color them = side_to_move;
    Color us   = (them == Color::WHITE ? Color::BLACK : Color::WHITE);

    // 1) restore clocks & side‐to‐move
    side_to_move      = us;
    castling_rights   = u.castling_rights;
    en_passant_square = u.en_passant_square;
    halfmove_clock    = u.halfmove_clock;
    fullmove_number   = u.fullmove_number;

    // 2) undo the piece move / promotion
    if (m.type == MoveType::PROMOTION) {
        // clear the promoted piece
        PieceIndex promoPiece = QUEEN;
        switch (m.promo) {
            case 'R': promoPiece = ROOK;   break;
            case 'B': promoPiece = BISHOP; break;
            case 'N': promoPiece = KNIGHT; break;
        }
        auto &promBB = (us == Color::WHITE ? whiteBB[promoPiece]
                                           : blackBB[promoPiece]);
        clearBit(promBB, m.end);

        // put the pawn back where it started
        auto &pawnBB = (us == Color::WHITE ? whiteBB[PAWN]
                                           : blackBB[PAWN]);
        setBit(pawnBB, m.start);
    } else {
        // normal move: take piece off 'end' and back onto 'start'
        auto &bb = (us == Color::WHITE ? whiteBB[u.moved_piece]
                                       : blackBB[u.moved_piece]);
        clearBit(bb, m.end);
        setBit(bb,   m.start);
    }

    // 3) restore any captured piece
    if (u.captured_piece < PieceTypeCount) {
        auto &capBB = (them == Color::WHITE ? whiteBB[u.captured_piece]
                                            : blackBB[u.captured_piece]);
        int restoreSq = (m.type == MoveType::EN_PASSANT)
                        ? (us == Color::WHITE ? m.end - 8 : m.end + 8)
                        : m.end;
        setBit(capBB, restoreSq);
    }

    // 4) undo castling rook‐move
    if (u.is_castling) {
        auto &rbb = (us == Color::WHITE ? whiteBB[ROOK]
                                        : blackBB[ROOK]);
        clearBit(rbb, u.castling_rook_to);
        setBit(rbb,   u.castling_rook_from);
    }

    // 5) pop your repetition history
    if (!positionHistory.empty())
        positionHistory.pop_back();
}


// ----------------------------------------------------------
//  Endgame detection
// ----------------------------------------------------------
bool Board::inCheck(Color c) const {
    int ksq = findKing(c);
    assert(ksq >= 0 && "no king on board when calling inCheck");
    // attacked by the opposite side
    Color attacker = (c == Color::WHITE ? Color::BLACK : Color::WHITE);
    return isSquareAttacked(ksq, attacker);
}

bool Board::hasLegalMoves(Color c) const {
    // temporarily switch side_to_move to c, generate legal moves, restore
    Color saved = side_to_move;
    const_cast<Board*>(this)->side_to_move = c;
    auto moves = generateLegalMoves();
    const_cast<Board*>(this)->side_to_move = saved;
    return !moves.empty();
}

bool Board::isCheckmate(Color c) const {
    return inCheck(c) && !hasLegalMoves(c);
}

bool Board::isStalemate(Color c) const {
    return !inCheck(c) && !hasLegalMoves(c);
}


bool Board::isFiftyMoveDraw() const {
    return halfmove_clock >= 100;
}

bool Board::isThreefoldRepetition() const {
    auto key = stripClocks(toFEN());
    int cnt = 0;
    for (auto &k : positionHistory)
        if (k == key && ++cnt >= 3)
            return true;
    return false;
}

bool Board::isInsufficientMaterial() const {
    // count all non-king material
    int minorCount = 0, majorCount = 0, pawnCount = 0;
    // white
    for (int pt = PAWN; pt < KING; ++pt) {
        uint64_t bbw = whiteBB[pt], bbb = blackBB[pt];
        int c = __builtin_popcountll(bbw) + __builtin_popcountll(bbb);
        if      (pt == PAWN)  pawnCount += c;
        else if (pt == KNIGHT || pt == BISHOP) minorCount += c;
        else /*ROOK,QUEEN*/  majorCount += c;
    }
    // if any pawn or any rook/queen, there is material
    if (pawnCount || majorCount) return false;
    // now only minors remain (and kings).  if there is at most one minor total, draw.
    return minorCount <= 1;
}


