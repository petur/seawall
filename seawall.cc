#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

enum Color : std::uint8_t { WHITE, BLACK };

inline Color operator~(Color c) { return static_cast<Color>(c ^ 1); }

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

enum Square : std::uint8_t { A1 = 0, H1 = 56, H8 = 63 };

inline Square& operator-=(Square& lhs, int rhs) { return lhs = static_cast<Square>(lhs - rhs); }
inline Square operator++(Square& lhs, int)
{
    Square r = lhs;
    lhs = static_cast<Square>(lhs + 1);
    return r;
}

Square parse_square(std::string_view s)
{
    return static_cast<Square>(8 * (s[0] - 'a') + (s[1] - '1'));
}

enum BitBoard : std::uint64_t { EMPTY = 0, ALL = ~0ULL };

inline BitBoard& operator|=(BitBoard& lhs, Square rhs) { return lhs = static_cast<BitBoard>(lhs | 1ULL << rhs); }
inline BitBoard& operator&=(BitBoard& lhs, BitBoard rhs) { return lhs = static_cast<BitBoard>(lhs & rhs); }
inline BitBoard operator~(Square sq) { return static_cast<BitBoard>(~(1ULL << sq)); }

enum Move : std::uint16_t { NULL_MOVE = 0, INVALID = 0xffff };

inline Move move(Square from, Square to, PieceType promotion = {}) { return static_cast<Move>(from | to << 6 | promotion << 12); }
inline Square from(Move mv) { return static_cast<Square>(mv & 63); }
inline Square to(Move mv) { return static_cast<Square>((mv >> 6) & 63); }

Move parse_move(std::string_view s)
{
    PieceType promotion{};
    if (s.length() > 4)
    {
        switch (s[4])
        {
            case 'n': promotion = KNIGHT; break;
            case 'b': promotion = BISHOP; break;
            case 'r': promotion = ROOK; break;
            case 'q': promotion = QUEEN; break;
        }
    }
    return move(parse_square(s.substr(0, 2)), parse_square(s.substr(2, 2)), promotion);
}

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
    if (memo.captured)
    {
        color_bb[color(memo.captured)] &= ~to(mv);
        type_bb[type(memo.captured)] &= ~to(mv);
    }
    Piece moved = squares[from(mv)];
    color_bb[color(moved)] &= ~from(mv);
    type_bb[type(moved)] &= ~from(mv);
    color_bb[color(moved)] |= to(mv);
    type_bb[type(moved)] |= to(mv);
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
    en_passant = parse_square(token);

    int fullmove_counter;
    fen >> halfmove_clock >> fullmove_counter;
}

const char startfen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
                position.do_move(parse_move(token));
            }
        }
        else if (token == "go")
        {

        }
        else if (token == "quit")
        {
            break;
        }
    }

    return 0;
}
