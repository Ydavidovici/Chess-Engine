// engine/src/board.cpp
#include "board.h"
#include <sstream>
#include <cassert>
#include <cctype>
#include <algorithm>

Board::Board() {
    initialize();
}

// 1) initialize()
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
    castling_rights   = 0b1111;  // KQkq
    en_passant_square = -1;
    halfmove_clock    = 0;
    fullmove_number   = 1;

    history.clear();
}

// 2) loadFEN(const std::string&)
void Board::loadFEN(const std::string& fen) {
    whiteBB.fill(0);
    blackBB.fill(0);

    std::istringstream iss(fen);
    std::string placement, stm, cr, ep;
    iss >> placement >> stm >> cr >> ep >> halfmove_clock >> fullmove_number;

    // Parse piece placement
    int rank = 7, file = 0;
    for (char c : placement) {
        if (c == '/') { --rank; file = 0; continue; }
        if (std::isdigit(c)) { file += c - '0'; continue; }
        int sq = rank*8 + file++;
        switch (c) {
        case 'P': setBit(whiteBB[PAWN],   sq); break;
        case 'N': setBit(whiteBB[KNIGHT], sq); break;
        case 'B': setBit(whiteBB[BISHOP], sq); break;
        case 'R': setBit(whiteBB[ROOK],   sq); break;
        case 'Q': setBit(whiteBB[QUEEN],  sq); break;
        case 'K': setBit(whiteBB[KING],   sq); break;
        case 'p': setBit(blackBB[PAWN],   sq); break;
        case 'n': setBit(blackBB[KNIGHT], sq); break;
        case 'b': setBit(blackBB[BISHOP], sq); break;
        case 'r': setBit(blackBB[ROOK],   sq); break;
        case 'q': setBit(blackBB[QUEEN],  sq); break;
        case 'k': setBit(blackBB[KING],   sq); break;
        }
    }

    // Side to move
    side_to_move = (stm == "w" ? Color::WHITE : Color::BLACK);

    // Castling rights
    castling_rights = 0;
    if (cr.find('K')!=std::string::npos) castling_rights |= 0b0001;
    if (cr.find('Q')!=std::string::npos) castling_rights |= 0b0010;
    if (cr.find('k')!=std::string::npos) castling_rights |= 0b0100;
    if (cr.find('q')!=std::string::npos) castling_rights |= 0b1000;

    // En passant target
    if (ep != "-") {
        int f = ep[0] - 'a', r = ep[1] - '1';
        en_passant_square = r*8 + f;
    } else {
        en_passant_square = -1;
    }
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

// For now, treat all pseudo-moves as legal.
std::vector<Move> Board::generateLegalMoves() const {
    return generatePseudoMoves();
}

// Full makeMove implementation
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

    // 8) Switch side and push history
    side_to_move = them;
    history.push_back(u);
    return true;
}

void Board::unmakeMove() {
    assert(!history.empty());
    Undo u = history.back();
    history.pop_back();

    Move m = u.move;
    Color them = side_to_move;
    Color us   = (them == Color::WHITE ? Color::BLACK : Color::WHITE);

    // Restore game state
    side_to_move      = us;
    castling_rights   = u.castling_rights;
    en_passant_square = u.en_passant_square;
    halfmove_clock    = u.halfmove_clock;
    fullmove_number   = u.fullmove_number;

    uint64_t fromMask = 1ULL << m.start;
    uint64_t toMask   = 1ULL << m.end;

    // Remove moved/promoted piece
    auto &dstBB = (us == Color::WHITE ? whiteBB[u.moved_piece] : blackBB[u.moved_piece]);
    clearBit(dstBB, m.end);

    // Restore pawn on promotion
    if (m.type == MoveType::PROMOTION && m.promo) {
        auto &pbb = (us == Color::WHITE ? whiteBB[PAWN] : blackBB[PAWN]);
        setBit(pbb, m.start);
    } else {
        setBit(dstBB, m.start);
    }

    // Restore captured piece
    if (u.captured_piece < PieceTypeCount) {
        auto &cbb = (them == Color::WHITE ? whiteBB[u.captured_piece]
                                          : blackBB[u.captured_piece]);
        if (m.type == MoveType::EN_PASSANT) {
            int epSq = (us == Color::WHITE ? m.end - 8 : m.end + 8);
            setBit(cbb, epSq);
        } else {
            setBit(cbb, m.end);
        }
    }

    // Undo castling rook move
    if (u.is_castling) {
        auto &rbb = (us == Color::WHITE ? whiteBB[ROOK] : blackBB[ROOK]);
        clearBit(rbb, u.castling_rook_to);
        setBit(rbb,   u.castling_rook_from);
    }
}
