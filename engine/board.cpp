// board.cpp
#include "board.h"
#include <sstream>
#include <vector>
#include <cctype>


Board::Board() {
    initialize();
}

void Board::initialize() {
    whitePawns   = 0x000000000000FF00ULL;
    whiteKnights = 0x0000000000000042ULL;
    whiteBishops = 0x0000000000000024ULL;
    whiteRooks   = 0x0000000000000081ULL;
    whiteQueens  = 0x0000000000000008ULL;
    whiteKings   = 0x0000000000000010ULL;

    blackPawns   = 0x00FF000000000000ULL;
    blackKnights = 0x4200000000000000ULL;
    blackBishops = 0x2400000000000000ULL;
    blackRooks   = 0x8100000000000000ULL;
    blackQueens  = 0x0800000000000000ULL;
    blackKings   = 0x1000000000000000ULL;
}

void Board::loadFEN(const std::string& fen) {
    // 1) Clear all bitboards
    whitePawns = whiteKnights = whiteBishops =
    whiteRooks = whiteQueens   = whiteKings   = 0;
    blackPawns = blackKnights = blackBishops =
    blackRooks = blackQueens   = blackKings   = 0;

    // 2) Extract placement before first space
    auto pos = fen.find(' ');
    std::string placement = fen.substr(0, pos);
    std::vector<std::string> rows;
    std::stringstream ss(placement);
    std::string row;
    while (std::getline(ss, row, '/')) rows.push_back(row);
    if (rows.size() != 8) return;

    // 3) Populate bitboards rank by rank
    for (int r = 0; r < 8; ++r) {
        const std::string& rowStr = rows[r];
        int file = 0, rank = 7 - r;
        for (char c : rowStr) {
            if (std::isdigit(c)) {
                file += c - '0';
            } else {
                int sq = rank*8 + file;
                switch (c) {
                    case 'P': set(whitePawns,   sq); break;
                    case 'N': set(whiteKnights, sq); break;
                    case 'B': set(whiteBishops, sq); break;
                    case 'R': set(whiteRooks,   sq); break;
                    case 'Q': set(whiteQueens,  sq); break;
                    case 'K': set(whiteKings,   sq); break;
                    case 'p': set(blackPawns,   sq); break;
                    case 'n': set(blackKnights, sq); break;
                    case 'b': set(blackBishops, sq); break;
                    case 'r': set(blackRooks,   sq); break;
                    case 'q': set(blackQueens,  sq); break;
                    case 'k': set(blackKings,   sq); break;
                }
                ++file;
            }
        }
    }
}

std::vector<std::string> Board::getBoardState() const {
    std::vector<std::string> board;
    board.reserve(8);
    for(int rank = 7; rank >= 0; --rank) {
        std::string line;
        line.reserve(16);
        for(int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            char p = '.';
            if      (bit(whitePawns,   sq)) p = 'P';
            else if (bit(whiteKnights, sq)) p = 'N';
            else if (bit(whiteBishops, sq)) p = 'B';
            else if (bit(whiteRooks,   sq)) p = 'R';
            else if (bit(whiteQueens,  sq)) p = 'Q';
            else if (bit(whiteKings,   sq)) p = 'K';
            else if (bit(blackPawns,   sq)) p = 'p';
            else if (bit(blackKnights, sq)) p = 'n';
            else if (bit(blackBishops, sq)) p = 'b';
            else if (bit(blackRooks,   sq)) p = 'r';
            else if (bit(blackQueens,  sq)) p = 'q';
            else if (bit(blackKings,   sq)) p = 'k';
            line.push_back(p);
            line.push_back(' ');
        }
        board.push_back(line);
    }
    return board;
}

bool Board::makeMove(const Move& m, Color c) {
    if (!m.isValid()) return false;
    uint64_t maskStart = 1ULL << m.start;
    uint64_t maskEnd   = 1ULL << m.end;

    // Setup arrays of pointers to our bitboards
    uint64_t* myBBs[6];
    uint64_t* opBBs[6];
    if (c == Color::WHITE) {
        myBBs[0] = &whitePawns;   myBBs[1] = &whiteKnights;
        myBBs[2] = &whiteBishops; myBBs[3] = &whiteRooks;
        myBBs[4] = &whiteQueens;  myBBs[5] = &whiteKings;
        opBBs[0] = &blackPawns;   opBBs[1] = &blackKnights;
        opBBs[2] = &blackBishops; opBBs[3] = &blackRooks;
        opBBs[4] = &blackQueens;  opBBs[5] = &blackKings;
    } else {
        myBBs[0] = &blackPawns;   myBBs[1] = &blackKnights;
        myBBs[2] = &blackBishops; myBBs[3] = &blackRooks;
        myBBs[4] = &blackQueens;  myBBs[5] = &blackKings;
        opBBs[0] = &whitePawns;   opBBs[1] = &whiteKnights;
        opBBs[2] = &whiteBishops; opBBs[3] = &whiteRooks;
        opBBs[4] = &whiteQueens;  opBBs[5] = &whiteKings;
    }

    // 1) Remove captured piece
    for (int i = 0; i < 6; ++i) {
        if (*opBBs[i] & maskEnd) {
            *opBBs[i] &= ~maskEnd;
            break;
        }
    }

    // 2) Handle promotion
    if (m.type == MoveType::PROMOTION && m.promo != '\0') {
        // clear pawn
        *myBBs[0] &= ~maskStart;
        // set promoted piece
        int idx = 4; // default Queen
        switch (m.promo) {
            case 'R': idx = 3; break;
            case 'B': idx = 2; break;
            case 'N': idx = 1; break;
        }
        *myBBs[idx] |= maskEnd;
        return true;
    }

    // 3) Move the piece
    for (int i = 0; i < 6; ++i) {
        if (*myBBs[i] & maskStart) {
            *myBBs[i] &= ~maskStart;
            *myBBs[i] |= maskEnd;
            return true;
        }
    }
    return false;
}

std::vector<Move> Board::generateLegalMoves(Color c) const {
    std::vector<Move> moves;

    // Occupancy
    uint64_t whiteOcc = whitePawns   | whiteKnights | whiteBishops |
                        whiteRooks   | whiteQueens  | whiteKings;
    uint64_t blackOcc = blackPawns   | blackKnights | blackBishops |
                        blackRooks   | blackQueens  | blackKings;
    uint64_t allOcc   = whiteOcc | blackOcc;
    uint64_t ownOcc   = (c == Color::WHITE) ? whiteOcc : blackOcc;
    uint64_t oppOcc   = (c == Color::WHITE) ? blackOcc : whiteOcc;

    // Directions
    static const int knightDirs[] = {-17,-15,-10,-6,6,10,15,17};
    static const int kingDirs[]   = {-9,-8,-7,-1,1,7,8,9};
    static const int bishopDirs[] = {-9,-7,7,9};
    static const int rookDirs[]   = {-8,-1,1,8};

    // Pick pawn bitboard & settings
    uint64_t pawnBB    = (c == Color::WHITE ? whitePawns : blackPawns);
    int      pawnDir   = (c == Color::WHITE ? 8 : -8);
    int      startRank= (c == Color::WHITE ? 1 : 6);
    int      promoRank= (c == Color::WHITE ? 7 : 0);

    // Pawn moves
    uint64_t tmp = pawnBB;
    while (tmp) {
        int sq = __builtin_ctzll(tmp);
        tmp &= tmp - 1;
        int t1 = sq + pawnDir;
        // single push
        if (t1>=0 && t1<64 && !(allOcc & (1ULL<<t1))) {
            if (t1/8 == promoRank) {
                for (char p: {'Q','R','B','N'})
                    moves.emplace_back(sq, t1, MoveType::PROMOTION, p);
            } else {
                moves.emplace_back(sq, t1);
                // double push
                if (sq/8 == startRank) {
                    int t2 = sq + 2*pawnDir;
                    if (t2>=0 && t2<64 && !(allOcc & (1ULL<<t2)))
                        moves.emplace_back(sq, t2);
                }
            }
        }
        // captures
        for (int d : {pawnDir-1, pawnDir+1}) {
            int tc = sq + d;
            if (tc>=0 && tc<64 && (oppOcc & (1ULL<<tc))) {
                if (tc/8 == promoRank) {
                    for (char p: {'Q','R','B','N'})
                        moves.emplace_back(sq, tc, MoveType::CAPTURE, p);
                } else {
                    moves.emplace_back(sq, tc, MoveType::CAPTURE);
                }
            }
        }
    }

    // Knight moves
    tmp = (c == Color::WHITE ? whiteKnights : blackKnights);
    for (; tmp; tmp &= tmp-1) {
        int sq = __builtin_ctzll(tmp);
        for (int d: knightDirs) {
            int t = sq + d;
            if (t>=0 && t<64 && !(ownOcc & (1ULL<<t))) {
                MoveType mt = (oppOcc & (1ULL<<t)) ? MoveType::CAPTURE : MoveType::NORMAL;
                moves.emplace_back(sq, t, mt);
            }
        }
    }

    // Sliding pieces (bishop, rook, queen)
    auto slide = [&](uint64_t bb, const int dirs[], int ndir){
        uint64_t tmp2 = bb;
        while (tmp2) {
            int sq = __builtin_ctzll(tmp2);
            tmp2 &= tmp2-1;
            for (int i=0;i<ndir;++i){
                int d = dirs[i], t = sq;
                while (true) {
                    t += d;
                    if (t<0||t>=64) break;
                    if (abs((sq%8)-(t%8))>2 && (d==-1||d==1||d==-9||d==-7||d==7||d==9)) break;
                    if (ownOcc & (1ULL<<t)) break;
                    MoveType mt = (oppOcc & (1ULL<<t)) ? MoveType::CAPTURE : MoveType::NORMAL;
                    moves.emplace_back(sq, t, mt);
                    if (allOcc & (1ULL<<t)) break;
                }
            }
        }
    };
    slide((c==Color::WHITE?whiteBishops:blackBishops), bishopDirs, 4);
    slide((c==Color::WHITE?whiteRooks  :blackRooks  ), rookDirs,   4);
    slide((c==Color::WHITE?whiteQueens :blackQueens ), bishopDirs, 4), slide((c==Color::WHITE?whiteQueens:blackQueens), rookDirs, 4);

    // King moves
    tmp = (c == Color::WHITE ? whiteKings : blackKings);
    for (; tmp; tmp &= tmp-1) {
        int sq = __builtin_ctzll(tmp);
        for (int d: kingDirs) {
            int t = sq + d;
            if (t>=0 && t<64 && !(ownOcc & (1ULL<<t))) {
                MoveType mt = (oppOcc & (1ULL<<t)) ? MoveType::CAPTURE : MoveType::NORMAL;
                moves.emplace_back(sq, t, mt);
            }
        }
    }

    return moves;
}