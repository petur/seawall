#include <cassert>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

enum Color : std::uint8_t { WHITE, BLACK };

inline Color operator~(Color c) { return static_cast<Color>(c ^ 1); }
inline int rank_fwd(Color c) { return c == WHITE ? 8 : -8; }

enum PieceType : std::uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

enum Piece : std::uint8_t
{
    NONE,
    WPAWN = 1 << 3, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
    BPAWN = 1 << 4, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING,
};

inline Color color(Piece p) { return static_cast<Color>(p >> 4); }
inline PieceType type(Piece p) { return static_cast<PieceType>(p & 7); }
inline Piece piece(Color c, PieceType t) { return static_cast<Piece>((c == WHITE ? 1 << 3 : 1 << 4) | t); }

enum Castling : std::uint8_t { WQ = 1 << 0, WK = 1 << 1, BQ = 1 << 2, BK = 1 << 3 };

inline Castling& operator|=(Castling& lhs, Castling rhs) { return lhs = static_cast<Castling>(lhs | rhs); }
inline Castling& operator&=(Castling& lhs, Castling rhs) { return lhs = static_cast<Castling>(lhs & rhs); }
inline Castling operator~(Castling c) { return static_cast<Castling>(~static_cast<std::uint8_t>(c)); }

std::ostream& operator<<(std::ostream& out, Castling c)
{
    if (c)
    {
        if (c & WK)
            out << 'K';
        if (c & WQ)
            out << 'Q';
        if (c & BK)
            out << 'k';
        if (c & BQ)
            out << 'q';
    }
    else
        out << '-';
    return out;
}

enum Square : std::uint8_t { A1 = 0, H1 = 56, H8 = 63, NO_SQUARE = 0xff };

inline Square& operator+=(Square& lhs, int rhs) { return lhs = static_cast<Square>(lhs + rhs); }
inline Square& operator-=(Square& lhs, int rhs) { return lhs = static_cast<Square>(lhs - rhs); }
inline Square operator++(Square& lhs, int)
{
    Square r = lhs;
    lhs = static_cast<Square>(lhs + 1);
    return r;
}

inline Square square(int file, int rank)
{
    return static_cast<Square>(file + (rank << 3));
}

Square parse_square(std::string_view s)
{
    return square(s[0] - 'a', s[1] - '1');
}

std::ostream& operator<<(std::ostream& out, Square sq)
{
    if (sq == NO_SQUARE)
        out << '-';
    else
        out << static_cast<char>('a' + (sq & 7)) << static_cast<char>('1' + (sq >> 3));
    return out;
}

enum BitBoard : std::uint64_t { EMPTY = 0, ALL = ~0ULL };

inline BitBoard bb(Square s) { return static_cast<BitBoard>(1ULL << s); }
inline BitBoard& operator|=(BitBoard& lhs, Square rhs) { return lhs = static_cast<BitBoard>(lhs | bb(rhs)); }
inline BitBoard& operator|=(BitBoard& lhs, BitBoard rhs) { return lhs = static_cast<BitBoard>(lhs | rhs); }
inline BitBoard& operator&=(BitBoard& lhs, BitBoard rhs) { return lhs = static_cast<BitBoard>(lhs & rhs); }
inline BitBoard operator|(BitBoard lhs, BitBoard rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) | rhs); }
inline BitBoard operator&(BitBoard lhs, BitBoard rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) & rhs); }
inline BitBoard operator<<(BitBoard lhs, int rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) << rhs); }
inline BitBoard operator>>(BitBoard lhs, int rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) >> rhs); }
inline BitBoard operator~(BitBoard b) { return static_cast<BitBoard>(~static_cast<std::uint64_t>(b)); }
inline BitBoard operator~(Square sq) { return ~bb(sq); }

inline Square first_square(BitBoard b)
{
    return static_cast<Square>(__builtin_ctzll(b));
}

inline Square pop(BitBoard& b)
{
    Square ret = first_square(b);
    b &= ~ret;
    return ret;
}

inline int popcount(BitBoard b) { return __builtin_popcountll(b); }

enum MoveType : std::uint8_t { REGULAR, EN_PASSANT = 6, CASTLING, CAPTURE, INVALID_TYPE = 15 };

inline MoveType& operator|=(MoveType& lhs, MoveType rhs) { return lhs = static_cast<MoveType>(lhs | rhs); }
inline MoveType& operator&=(MoveType& lhs, MoveType rhs) { return lhs = static_cast<MoveType>(lhs & rhs); }
inline MoveType operator|(MoveType lhs, MoveType rhs) { return lhs |= rhs; }
inline MoveType operator&(MoveType lhs, MoveType rhs) { return lhs &= rhs; }
inline MoveType operator~(MoveType m) { return static_cast<MoveType>(~static_cast<unsigned>(m)); }

enum Move : std::uint16_t { NULL_MOVE = 0, INVALID = 0xffff };

inline Move move(Square from, Square to, MoveType type) { return static_cast<Move>(from | to << 6 | type << 12); }
inline Square from(Move mv) { return static_cast<Square>(mv & 63); }
inline Square to(Move mv) { return static_cast<Square>((mv >> 6) & 63); }
inline MoveType type(Move mv) { return static_cast<MoveType>((mv >> 12) & 15); }

std::ostream& operator<<(std::ostream& out, Move mv)
{
    out << from(mv) << to(mv);
    switch (static_cast<PieceType>(type(mv) & ~CAPTURE))
    {
        case QUEEN: out << 'q'; break;
        case ROOK: out << 'r'; break;
        case BISHOP: out << 'b'; break;
        case KNIGHT: out << 'n'; break;
        default: break;
    }
    return out;
}

struct Memo
{
    Piece moved;
    Piece captured;
    Castling castling;
    Square en_passant;
    int halfmove_clock;
};

struct Position
{
    Memo do_move(Move mv);
    void undo_move(Move mv, const Memo& memo);

    void parse(std::istream& fen);
    Move parse_move(std::string_view s) const;

    void debug(std::ostream& out);

    BitBoard all_bb() const { return color_bb[WHITE] | color_bb[BLACK]; }

    void clear(Square sq, Piece p) { clear(sq, color(p), type(p)); }
    void clear(Square sq, Color c, PieceType t);
    void set(Square sq, Piece p) { set(sq, color(p), type(p), p); }
    void set(Square sq, Color c, PieceType t) { set(sq, c, t, piece(c, t)); }
    void set(Square sq, Color c, PieceType t, Piece p);

    Piece squares[64];
    BitBoard color_bb[2];
    BitBoard type_bb[6];
    Color next;
    Castling castling;
    Square en_passant;
    int halfmove_clock;
} position;

void Position::clear(Square sq, Color c, PieceType t)
{
    color_bb[c] &= ~sq;
    type_bb[t] &= ~sq;
    squares[sq] = NONE;
}

void Position::set(Square sq, Color c, PieceType t, Piece p)
{
    assert(t >= PAWN && t <= KING);
    color_bb[c] |= sq;
    type_bb[t] |= sq;
    squares[sq] = p;
}

Memo Position::do_move(Move mv)
{
    Memo memo{squares[from(mv)], squares[to(mv)], castling, en_passant, halfmove_clock};
    ++halfmove_clock;

    MoveType mt = type(mv);
    if (mt & CAPTURE)
    {
        Square cap_sq = to(mv);
        if (mt == (EN_PASSANT | CAPTURE))
        {
            cap_sq = static_cast<Square>(cap_sq + rank_fwd(~next));
            memo.captured = squares[cap_sq];
        }

        clear(cap_sq, memo.captured);
        halfmove_clock = 0;
    }
    Piece moved = squares[from(mv)];
    clear(from(mv), moved);
    MoveType promotion = mt & ~CAPTURE;
    if (promotion && promotion <= static_cast<MoveType>(QUEEN))
        set(to(mv), next, static_cast<PieceType>(promotion));
    else
        set(to(mv), moved);
    en_passant = NO_SQUARE;

    if (type(moved) == PAWN)
    {
        halfmove_clock = 0;
        if (mt == EN_PASSANT)
            en_passant = static_cast<Square>(from(mv) + rank_fwd(next));
    }
    else if (mt == CASTLING)
    {
        int rank = from(mv) & ~7;
        Square rook_from = static_cast<Square>(rank | (to(mv) < from(mv) ? 0 : 7));
        Square rook_to = static_cast<Square>(rank | (to(mv) < from(mv) ? 3 : 5));
        clear(rook_from, next, ROOK);
        set(rook_to, next, ROOK);
        castling &= ~static_cast<Castling>(3 << (2 * next));
    }
    else if (castling)
    {
        if (type(moved) == KING)
            castling &= ~static_cast<Castling>(3 << (2 * next));
        else if (type(moved) == ROOK)
            castling &= ~static_cast<Castling>(((from(mv) & 1) ? WK : WQ) << (2 * next));
        if ((type(mv) & CAPTURE) && type(memo.captured) == ROOK)
            castling &= ~static_cast<Castling>(((to(mv) & 1) ? WK : WQ) << (2 * ~next));
    }

    next = ~next;
    return memo;
}

void Position::undo_move(Move mv, const Memo& memo)
{
    next = ~next;
    halfmove_clock = memo.halfmove_clock;
    en_passant = memo.en_passant;
    castling = memo.castling;

    clear(to(mv), squares[to(mv)]);
    set(from(mv), memo.moved);

    MoveType mt = type(mv);
    if (mt & CAPTURE)
    {
        Square cap_sq = to(mv);
        if (mt == (EN_PASSANT | CAPTURE))
            cap_sq = static_cast<Square>(cap_sq + rank_fwd(~next));
        set(cap_sq, memo.captured);
    }

    if (mt == CASTLING)
    {
        int rank = from(mv) & ~7;
        Square rook_from = static_cast<Square>(rank | (to(mv) < from(mv) ? 0 : 7));
        Square rook_to = static_cast<Square>(rank | (to(mv) < from(mv) ? 3 : 5));
        clear(rook_to, next, ROOK);
        set(rook_from, next, ROOK);
    }
}

void Position::parse(std::istream& fen)
{
    std::string token;
    fen >> token;

    for (BitBoard& b : color_bb)
        b = EMPTY;
    for (BitBoard& b : type_bb)
        b = EMPTY;
    for (Piece& p : squares)
        p = NONE;
    Square sq = H1;

    for (char ch : token)
    {
        if (ch == '/')
            sq -= 16;
        else if (ch >= '1' && ch <= '8')
            sq += ch - '0';
        else
        {
            Piece p;
            switch (ch)
            {
                case 'P': p = WPAWN; break;
                case 'N': p = WKNIGHT; break;
                case 'B': p = WBISHOP; break;
                case 'R': p = WROOK; break;
                case 'Q': p = WQUEEN; break;
                case 'K': p = WKING; break;
                case 'p': p = BPAWN; break;
                case 'n': p = BKNIGHT; break;
                case 'b': p = BBISHOP; break;
                case 'r': p = BROOK; break;
                case 'q': p = BQUEEN; break;
                case 'k': p = BKING; break;
            }
            set(sq++, p);
        }
    }

    fen >> token;
    if (token == "w")
        next = WHITE;
    else if (token == "b")
        next = BLACK;

    fen >> token;
    castling = Castling{};
    for (char ch : token)
    {
        switch (ch)
        {
            case 'K': castling |= WK; break;
            case 'Q': castling |= WQ; break;
            case 'k': castling |= BK; break;
            case 'q': castling |= BQ; break;
        }
    }

    fen >> token;
    if (token == "-")
        en_passant = NO_SQUARE;
    else
        en_passant = parse_square(token);

    int fullmove_counter;
    fen >> halfmove_clock >> fullmove_counter;
}

Move Position::parse_move(std::string_view s) const
{
    Square from = parse_square(s.substr(0, 2));
    Square to = parse_square(s.substr(2, 2));

    MoveType mt{};
    if (s.length() > 4)
    {
        PieceType promotion{};
        switch (s[4])
        {
            case 'n': promotion = KNIGHT; break;
            case 'b': promotion = BISHOP; break;
            case 'r': promotion = ROOK; break;
            case 'q': promotion = QUEEN; break;
        }
        mt = static_cast<MoveType>(promotion);
    }
    else if (type(squares[from]) == PAWN)
    {
        if (to == en_passant)
            mt |= EN_PASSANT | CAPTURE;
        else if (std::abs(from - to) == 16)
            mt |= EN_PASSANT;
    }
    else if (type(squares[from]) == KING)
    {
        if (std::abs((from & 7) - (to & 7)) > 1)
            mt |= CASTLING;
    }

    if (squares[to])
        mt |= CAPTURE;
    return move(from, to, mt);
}

void Position::debug(std::ostream& out)
{
    out << "info string fen ";

    for (int rank = 7; rank >= 0; --rank)
    {
        int blanks = 0;

        for (int file = 0; file < 8; ++file)
        {
            Square sq = square(file, rank);
            if (squares[sq] == NONE)
            {
                ++blanks;
            }
            else
            {
                if (blanks)
                {
                    out << blanks;
                    blanks = 0;
                }
                switch (squares[sq])
                {
                    case WPAWN: out << 'P'; break;
                    case WKNIGHT: out << 'N'; break;
                    case WBISHOP: out << 'B'; break;
                    case WROOK: out << 'R'; break;
                    case WQUEEN: out << 'Q'; break;
                    case WKING: out << 'K'; break;
                    case BPAWN: out << 'p'; break;
                    case BKNIGHT: out << 'n'; break;
                    case BBISHOP: out << 'b'; break;
                    case BROOK: out << 'r'; break;
                    case BQUEEN: out << 'q'; break;
                    case BKING: out << 'k'; break;
                    default: break;
                }
            }
        }

        if (blanks)
        {
            out << blanks;
            blanks = 0;
        }
        if (rank > 0)
            out << '/';
    }

    out << ' ' << (next == WHITE ? 'w' : 'b') << ' ' << castling << ' ' << en_passant << ' ' << halfmove_clock << " 1\n";
}

BitBoard knight_attack[64];
BitBoard king_attack[64];
BitBoard pawn_attack[2][64];
BitBoard pawn_push[2][64];
BitBoard pawn_double_push[2][64];
BitBoard ray[64][8];

BitBoard offset_bitboard(int file, int rank, const std::pair<int, int> (&offsets)[8])
{
    BitBoard ret{};
    for (auto off : offsets)
    {
        if (file + off.first >= 0 && file + off.first <= 7 && rank + off.second >= 0 && rank + off.second <= 7)
            ret |= square(file + off.first, rank + off.second);
    }
    return ret;
}

void init_bitboards()
{
    constexpr std::pair<int, int> knight_offsets[8] = {{-2, -1}, {-1, -2}, {2, -1}, {-1, 2}, {-2, 1}, {1, -2}, {2, 1}, {1, 2}};
    constexpr std::pair<int, int> king_offsets[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    constexpr std::pair<int, int> ray_offsets[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}};

    for (Square sq = A1; sq <= H8; sq++)
    {
        int file = sq & 7;
        int rank = sq >> 3;

        knight_attack[sq] = offset_bitboard(file, rank, knight_offsets);
        king_attack[sq] = offset_bitboard(file, rank, king_offsets);

        for (Color c : {WHITE, BLACK})
        {
            if (rank == (c == WHITE ? 7 : 0))
                continue;
            if (file > 0)
                pawn_attack[c][sq] |= static_cast<Square>(sq - 1 + rank_fwd(c));
            if (file < 7)
                pawn_attack[c][sq] |= static_cast<Square>(sq + 1 + rank_fwd(c));

            pawn_push[c][sq] = bb(static_cast<Square>(sq + rank_fwd(c)));

            if (rank == (c == WHITE ? 1 : 6))
                pawn_double_push[c][sq] = bb(static_cast<Square>(sq + 2 * rank_fwd(c)));
        }

        for (int i = 0; i < 8; ++i)
        {
            auto off = ray_offsets[i];
            int f = file + off.first;
            int r = rank + off.second;
            while (f >= 0 && f <= 7 && r >= 0 && r <= 7)
            {
                ray[sq][i] |= square(f, r);
                f += off.first;
                r += off.second;
            }
        }
    }
}

template <int N> BitBoard shift_signed(BitBoard b)
{
    if (N > 0)
        return b << N;
    else
        return b >> -N;
}

template <int Shift> BitBoard ray_attack(BitBoard ray, BitBoard blockers)
{
    BitBoard v = ray & blockers;
    v = shift_signed<Shift>(v);
    v |= shift_signed<Shift>(v);
    v |= shift_signed<Shift * 2>(v);
    v |= shift_signed<Shift * 4>(v);
    return ray & ~v;
}

BitBoard bishop_attack(Square sq, BitBoard blockers)
{
    return ray_attack<-9>(ray[sq][0], blockers)
        | ray_attack<7>(ray[sq][2], blockers)
        | ray_attack<9>(ray[sq][4], blockers)
        | ray_attack<-7>(ray[sq][6], blockers);
}

BitBoard rook_attack(Square sq, BitBoard blockers)
{
    return ray_attack<-1>(ray[sq][1], blockers)
        | ray_attack<8>(ray[sq][3], blockers)
        | ray_attack<1>(ray[sq][5], blockers)
        | ray_attack<-8>(ray[sq][7], blockers);
}

BitBoard queen_attack(Square sq, BitBoard blockers)
{
    return bishop_attack(sq, blockers) | rook_attack(sq, blockers);
}

BitBoard attackers(Square sq, Color c)
{
    return position.color_bb[c] & (
        (pawn_attack[~c][sq] & position.type_bb[PAWN])
        | (knight_attack[sq] & position.type_bb[KNIGHT])
        | (bishop_attack(sq, position.all_bb()) & position.type_bb[BISHOP])
        | (rook_attack(sq, position.all_bb()) & position.type_bb[ROOK])
        | (queen_attack(sq, position.all_bb()) & position.type_bb[QUEEN])
        | (king_attack[sq] & position.type_bb[KING])
    );
}

struct MoveGen
{
    Move moves[256];
    int count;
    int index;

    Move next();

    void generate();
    template<PieceType Type> void generate_pieces();
    template<PieceType Type> void generate_piece(Square sq);
    void generate_targets(Square sq, BitBoard targets);
    void generate_targets(Square sq, BitBoard targets, MoveType mt);
    void generate_pawn_targets(Square sq, BitBoard targets, MoveType mt);
    void generate_target(Square sq, Square target, MoveType mt);
    template<int Offset> void generate_castling(Square sq);
};

void MoveGen::generate_target(Square sq, Square target, MoveType mt)
{
    assert(!(target & ~63));
    moves[count++] = move(sq, target, mt);
}

void MoveGen::generate_targets(Square sq, BitBoard targets, MoveType mt)
{
    while (targets)
        generate_target(sq, pop(targets), mt);
}

void MoveGen::generate_pawn_targets(Square sq, BitBoard targets, MoveType mt)
{
    if ((sq >> 3) == (position.next == WHITE ? 6 : 1))
    {
        generate_targets(sq, targets, static_cast<MoveType>(mt | QUEEN));
        generate_targets(sq, targets, static_cast<MoveType>(mt | ROOK));
        generate_targets(sq, targets, static_cast<MoveType>(mt | BISHOP));
        generate_targets(sq, targets, static_cast<MoveType>(mt | KNIGHT));
    }
    else
        generate_targets(sq, targets, mt);
}

void MoveGen::generate_targets(Square sq, BitBoard targets)
{
    generate_targets(sq, targets & position.color_bb[~position.next], CAPTURE);
    generate_targets(sq, targets & ~position.all_bb(), {});
}

template<int Offset> void MoveGen::generate_castling(Square sq)
{
    int rank = sq >> 3;
    Square rook_from = square(Offset < 0 ? 0 : 7, rank);
    if (!(ray_attack<Offset>(ray[sq][Offset < 0 ? 1 : 5], position.all_bb()) & rook_from))
        return;

    for (int i = 0; i <= 2; i++)
    {
        Square s = square(4 + i * Offset, rank);
        if (attackers(s, ~position.next))
            return;
    }

    generate_target(sq, square(4 + 2 * Offset, rank), CASTLING);
}

template<PieceType Type> void MoveGen::generate_piece(Square sq)
{
    if (Type == PAWN)
    {
        generate_pawn_targets(sq, pawn_attack[position.next][sq] & position.color_bb[~position.next], CAPTURE);
        BitBoard push = pawn_push[position.next][sq] & ~position.all_bb();
        if (push)
        {
            generate_pawn_targets(sq, push, {});
            generate_targets(sq, pawn_double_push[position.next][sq] & ~position.all_bb(), EN_PASSANT);
        }
        if (position.en_passant != NO_SQUARE && (pawn_attack[position.next][sq] & position.en_passant))
            generate_target(sq, position.en_passant, EN_PASSANT | CAPTURE);
    }
    else if (Type == KNIGHT)
        generate_targets(sq, knight_attack[sq]);
    else if (Type == BISHOP)
        generate_targets(sq, bishop_attack(sq, position.all_bb()));
    else if (Type == ROOK)
        generate_targets(sq, rook_attack(sq, position.all_bb()));
    else if (Type == QUEEN)
        generate_targets(sq, queen_attack(sq, position.all_bb()));
    else if (Type == KING)
    {
        generate_targets(sq, king_attack[sq]);
        if (position.castling & (WQ << (2 * position.next)))
            generate_castling<-1>(sq);
        if (position.castling & (WK << (2 * position.next)))
            generate_castling<1>(sq);
    }
}

template<PieceType Type> void MoveGen::generate_pieces()
{
    BitBoard pieces = position.color_bb[position.next] & position.type_bb[Type];
    while (pieces)
        generate_piece<Type>(pop(pieces));
}

void MoveGen::generate()
{
    generate_pieces<PAWN>();
    generate_pieces<KNIGHT>();
    generate_pieces<BISHOP>();
    generate_pieces<ROOK>();
    generate_pieces<QUEEN>();
    generate_pieces<KING>();
}

Move MoveGen::next()
{
    if (count == 0)
        generate();
    if (index >= count)
        return NULL_MOVE;
    return moves[index++];
}

int material(Color c)
{
    return 100 * popcount(position.type_bb[PAWN] & position.color_bb[c])
        + 300 * popcount(position.type_bb[KNIGHT] & position.color_bb[c])
        + 300 * popcount(position.type_bb[BISHOP] & position.color_bb[c])
        + 500 * popcount(position.type_bb[ROOK] & position.color_bb[c])
        + 900 * popcount(position.type_bb[QUEEN] & position.color_bb[c])
        + 100000 * popcount(position.type_bb[KING] & position.color_bb[c]);
}

int evaluate()
{
    return material(position.next) - material(~position.next);
}

struct Search
{
    std::istream& in;
    std::clock_t target_time;
    std::clock_t max_time;
    std::clock_t start;
    long long nodes;
    bool stopped;

    Search(std::istream& i, std::clock_t time, std::clock_t inc, std::clock_t movetime);

    bool is_stopped(bool max);

    std::pair<int, Move> search(int ply, int depth, int alpha, int beta);
    void iterate(std::ostream& out, int max_depth);
};

Search::Search(std::istream& i, std::clock_t time, std::clock_t inc, std::clock_t movetime)
    : in{i}, target_time{std::numeric_limits<std::clock_t>::max()}, max_time{std::numeric_limits<std::clock_t>::max()},
    start{std::clock()}, nodes{}, stopped{}
{
    if (movetime != -1)
        target_time = max_time = movetime;
    else if (time != -1)
    {
        int pieces = popcount(position.color_bb[WHITE] | position.color_bb[BLACK]);
        time -= std::min(time / 8, CLOCKS_PER_SEC / 4);
        max_time = std::min(time, time / (2 + pieces / 8) + inc);
        target_time = std::min(max_time, time / (16 + 2 * pieces) + inc / 4);
    }
}

bool Search::is_stopped(bool max)
{
    if (!stopped && (!max || (nodes & 0xff) == 0))
    {
        if (std::clock() - start > (max ? max_time : target_time))
            stopped = true;
        else if ((nodes & 0xffff) == 0 && in.rdbuf()->in_avail())
        {
            std::string token;
            in >> token;
            if (token == "stop")
                stopped = true;
        }
    }
    return stopped;
}

std::pair<int, Move> Search::search(int ply, int depth, int alpha, int beta)
{
    MoveGen gen{};
    Move best = NULL_MOVE;

    Square king_sq = first_square(position.type_bb[KING] & position.color_bb[position.next]);

    int move_count = 0;
    while (Move mv = gen.next())
    {
        ++nodes;
        ++move_count;
        Memo memo = position.do_move(mv);

        int v;
        if (attackers(from(mv) == king_sq ? to(mv) : king_sq, position.next))
        {
            v = -32767;
            --move_count;
        }
        else if (depth <= 1)
        {
            v = -evaluate();
        }
        else
            v = -search(ply + 1, depth - 1, -beta, -alpha).first;

        position.undo_move(mv, memo);

        if (v > alpha)
        {
            alpha = v;
            best = mv;
        }
        if (is_stopped(true))
            return {alpha, best};
        if (alpha > beta)
            return {beta, mv};
    }

    if (move_count == 0)
        return {attackers(king_sq, ~position.next) ? -32767 + ply : 0, NULL_MOVE};

    return {alpha, best};
}

void Search::iterate(std::ostream& out, int max_depth)
{
    std::pair<int, Move> best{};
    for (int depth = 1; depth <= max_depth; ++depth)
    {
        auto v = search(0, depth, -32767, 32767);
        if (!stopped || !best.second)
            best = v;

        std::clock_t now = std::clock();
        out << "info depth " << depth << " score ";
        if (best.first > 32000)
            out << "mate " << ((32767 - best.first + 1) / 2);
        else if (best.first < -32000)
            out << "mate " << ((-32767 - best.first) / 2);
        else
            out << "cp " << best.first;
        out << " nodes " << nodes
            << std::fixed << std::setprecision(0) << " time " << (static_cast<double>(now - start) * 1000. / CLOCKS_PER_SEC)
            << " nps " << (nodes * CLOCKS_PER_SEC / static_cast<double>(now - start)) << " pv " << best.second << std::endl;

        if (is_stopped(false))
            break;
    }
    out << "bestmove " << best.second << std::endl;
}

const char startfen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

bool debug;

int main()
{
    std::ios::sync_with_stdio(false);
    std::string line;

    while (getline(std::cin, line))
    {
        std::istringstream parser{line};
        std::string token;
        parser >> token;

        if (token == "uci")
        {
            std::cout
                << "id name seawall\n"
                << "id author petur\n"
                << "uciok" << std::endl;
        }
        else if (token == "debug")
        {
            parser >> token;
            if (token == "on")
                debug = true;
            else if (token == "off")
                debug = false;
        }
        else if (token == "isready")
        {
            if (!king_attack[A1])
                init_bitboards();
            std::cout << "readyok" << std::endl;
        }
        else if (token == "position")
        {
            parser >> token;
            if (token == "startpos")
            {
                std::istringstream start{startfen};
                position.parse(start);
            }
            else
                position.parse(parser);

            parser >> token;
            while (parser >> token)
            {
                position.do_move(position.parse_move(token));
            }
            if (debug)
                position.debug(std::cout);
        }
        else if (token == "go")
        {
            int max_depth = 128;
            std::clock_t time = -1;
            std::clock_t inc = 0;
            std::clock_t movetime = -1;

            while (parser >> token)
            {
                if (token == "depth")
                    parser >> max_depth;
                if (token == (position.next == WHITE ? "wtime" : "btime"))
                {
                    long millis;
                    parser >> millis;
                    time = millis * CLOCKS_PER_SEC / 1000L;
                }
                if (token == (position.next == WHITE ? "winc" : "binc"))
                {
                    long millis;
                    parser >> millis;
                    inc = millis * CLOCKS_PER_SEC / 1000L;
                }
                if (token == "movetime")
                {
                    long millis;
                    parser >> millis;
                    movetime = millis * CLOCKS_PER_SEC / 1000L;
                }
            }

            Search{std::cin, time, inc, movetime}.iterate(std::cout, max_depth);
        }
        else if (token == "quit")
        {
            break;
        }
    }

    return 0;
}
