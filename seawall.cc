#include <iostream>
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

inline Square& operator-=(Square& lhs, int rhs) { return lhs = static_cast<Square>(lhs - rhs); }
inline Square operator++(Square& lhs, int)
{
    Square r = lhs;
    lhs = static_cast<Square>(lhs + 1);
    return r;
}

Square parse_square(std::string_view s)
{
    return static_cast<Square>((s[0] - 'a') + ((s[1] - '1') << 3));
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

inline BitBoard& operator|=(BitBoard& lhs, Square rhs) { return lhs = static_cast<BitBoard>(lhs | 1ULL << rhs); }
inline BitBoard& operator&=(BitBoard& lhs, BitBoard rhs) { return lhs = static_cast<BitBoard>(lhs & rhs); }
inline BitBoard operator~(Square sq) { return static_cast<BitBoard>(~(1ULL << sq)); }

enum MoveType : std::uint8_t { REGULAR, EN_PASSANT = 6, CASTLING, CAPTURE, INVALID_TYPE = 15 };

inline MoveType& operator|=(MoveType& lhs, MoveType rhs) { return lhs = static_cast<MoveType>(lhs | rhs); }
inline MoveType operator|(MoveType lhs, MoveType rhs) { return lhs |= rhs; }

enum Move : std::uint16_t { NULL_MOVE = 0, INVALID = 0xffff };

inline Move move(Square from, Square to, MoveType type) { return static_cast<Move>(from | to << 6 | type << 12); }
inline Square from(Move mv) { return static_cast<Square>(mv & 63); }
inline Square to(Move mv) { return static_cast<Square>((mv >> 6) & 63); }
inline MoveType type(Move mv) { return static_cast<MoveType>((mv >> 12) & 15); }

struct Memo
{
    Piece captured;
    Castling castling;
    Square en_passant;
    int halfmove_clock;
};

struct Position
{
    Memo do_move(Move mv);

    void parse(std::istream& fen);
    Move parse_move(std::string_view s) const;

    void debug(std::ostream& out);

    Piece squares[64];
    BitBoard color_bb[2];
    BitBoard type_bb[6];
    Color next;
    Castling castling;
    Square en_passant;
    int halfmove_clock;
} position;

Memo Position::do_move(Move mv)
{
    Memo memo{squares[to(mv)], castling, en_passant, halfmove_clock};
    ++halfmove_clock;

    if (type(mv) & CAPTURE)
    {
        Square cap_sq = to(mv);
        if (type(mv) & EN_PASSANT)
        {
            cap_sq = static_cast<Square>(cap_sq + rank_fwd(~next));
            memo.captured = squares[cap_sq];
            squares[cap_sq] = NONE;
        }

        color_bb[color(memo.captured)] &= ~cap_sq;
        type_bb[type(memo.captured)] &= ~cap_sq;
        halfmove_clock = 0;
    }
    Piece moved = squares[from(mv)];
    color_bb[color(moved)] &= ~from(mv);
    type_bb[type(moved)] &= ~from(mv);
    color_bb[color(moved)] |= to(mv);
    type_bb[type(moved)] |= to(mv);
    squares[from(mv)] = NONE;
    squares[to(mv)] = moved;
    en_passant = NO_SQUARE;

    if (type(moved) == PAWN)
    {
        halfmove_clock = 0;
        if ((type(mv) & EN_PASSANT) && !(type(mv) & CAPTURE))
            en_passant = static_cast<Square>(from(mv) + rank_fwd(next));
    }
    else if (type(mv) & CASTLING)
    {
        int rank = from(mv) & ~7;
        Square rook_from = static_cast<Square>(rank | (to(mv) < from(mv) ? 0 : 7));
        Square rook_to = static_cast<Square>(rank | (to(mv) < from(mv) ? 3 : 5));
        color_bb[next] |= rook_to;
        type_bb[ROOK] |= rook_to;
        squares[rook_to] = squares[rook_from];
        color_bb[next] &= ~rook_from;
        type_bb[ROOK] &= ~rook_from;
        squares[rook_from] = NONE;
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

void Position::parse(std::istream& fen)
{
    std::string token;
    fen >> token;

    for (BitBoard& b : color_bb)
        b = EMPTY;
    for (BitBoard& b : type_bb)
        b = EMPTY;
    Square sq = H1;

    for (char ch : token)
    {
        if (ch == '/')
            sq -= 16;
        else if (ch >= '1' && ch <= '8')
        {
            for (int i = 0; i < ch - '0'; i++)
                squares[sq++] = NONE;
        }
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
            color_bb[color(p)] |= sq;
            type_bb[type(p)] |= sq;
            squares[sq++] = p;
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
            Square sq = static_cast<Square>(8 * rank + file);
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

void search(int ply, int depth)
{
    (void) ply;
    (void) depth;
}

void iterate()
{
    for (int depth = 1; ; ++depth)
        search(0, depth);
}

const char startfen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

bool debug;

int main()
{
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
            iterate();
        }
        else if (token == "quit")
        {
            break;
        }
    }

    return 0;
}
