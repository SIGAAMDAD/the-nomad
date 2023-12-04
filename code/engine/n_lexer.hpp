class Token
{
public:
    enum class Kind {
        Number,
        Identifier,
        LeftParen,
        RightParen,
        LeftSquare,
        RightSquare,
        LeftCurly,
        RightCurly,
        LessThan,
        GreaterThan,
        Equal,
        Plus,
        Minus,
        Asterisk,
        Slash,
        Hash,
        Dot,
        Comma,
        Colon,
        Semicolon,
        SingleQuote,
        DoubleQuote,
        Comment,
        Pipe,
        End,
        Unexpected,
        NewLine,
    };

    GDR_INLINE Token() = default;
    GDR_INLINE Token(Kind kind)
        : m_kind(kind) { }
    GDR_INLINE Token(Kind kind, char* beg, size_t len)
        : m_kind(kind), m_len(len), m_lexeme(beg) { }
    GDR_INLINE Token(Kind kind, char* beg, char* end)
        : m_kind(kind), m_len(std::distance(beg, end)), m_lexeme(beg) { }

    GDR_INLINE Kind kind() const { return m_kind; }
    GDR_INLINE void kind(Kind kind) { m_kind = kind; }
    GDR_INLINE bool is(Kind kind) const { return m_kind == kind; }
    GDR_INLINE bool is_not(Kind kind) const { return m_kind != kind; }
    GDR_INLINE bool is_one_of(Kind k1, Kind k2) const { return is(k1) || is(k2); }

    template <typename... Ts>
    GDR_INLINE bool is_one_of(Kind k1, Kind k2, Ts... ks) const { return is(k1) || is_one_of(k2, ks...); }

    GDR_INLINE char* lexeme(void) { return m_lexeme; }
    GDR_INLINE void lexeme(char *lexeme) { m_lexeme = lexeme; }
    GDR_INLINE uint64_t length(void) const { return m_len; }
private:
    Kind m_kind;
    uint64_t m_len;
    char *m_lexeme;
};

class Lexer
{
public:
    GDR_INLINE Lexer(char* beg)
        : m_beg(beg) { }
    GDR_INLINE ~Lexer() = default;
    Token next();
private:
    Token identifier();
    Token number();
    Token slash_or_comment();
    Token atom(Token::Kind);

    char peek() const { return *m_beg; }
    char get() { return *m_beg++; }

    char* m_beg = NULL;
};

GDR_INLINE bool is_space(char c)
{
    switch (c) {
    case ' ':
    case '\t':
    case '\r': return true;
    default:   return false;
    };
}

GDR_INLINE bool is_digit(char c)
{
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': return true;
    default:  return false;
    };
}

GDR_INLINE bool is_identifier_char(char c)
{
    switch (c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '_': return true;
    default:  return false;
    };
}

GDR_INLINE Token Lexer::atom(Token::Kind kind) { return Token(kind, m_beg++, 1); }

GDR_INLINE Token Lexer::next()
{
    while (is_space(peek())) get();

    switch (peek()) {
    case '\0': return Token(Token::Kind::End, m_beg, 1);
    case '\n': return atom(Token::Kind::NewLine);
    default:   return atom(Token::Kind::Unexpected);
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':  return identifier();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':  return number();
    case '(':  return atom(Token::Kind::LeftParen);
    case ')':  return atom(Token::Kind::RightParen);
    case '[':  return atom(Token::Kind::LeftSquare);
    case ']':  return atom(Token::Kind::RightSquare);
    case '{':  return atom(Token::Kind::LeftCurly);
    case '}':  return atom(Token::Kind::RightCurly);
    case '<':  return atom(Token::Kind::LessThan);
    case '>':  return atom(Token::Kind::GreaterThan);
    case '=':  return atom(Token::Kind::Equal);
    case '+':  return atom(Token::Kind::Plus);
    case '-':  return atom(Token::Kind::Minus);
    case '*':  return atom(Token::Kind::Asterisk);
    case '/':  return slash_or_comment();
    case '#':  return atom(Token::Kind::Hash);
    case '.':  return atom(Token::Kind::Dot);
    case ',':  return atom(Token::Kind::Comma);
    case ':':  return atom(Token::Kind::Colon);
    case ';':  return atom(Token::Kind::Semicolon);
    case '\'': return atom(Token::Kind::SingleQuote);
    case '"':  return atom(Token::Kind::DoubleQuote);
    case '|':  return atom(Token::Kind::Pipe);
    };
}

GDR_INLINE Token Lexer::identifier()
{
    char* start = m_beg;
    get();
    while (is_identifier_char(peek())) get();
    return Token(Token::Kind::Identifier, start, m_beg);
}

GDR_INLINE Token Lexer::number()
{
    char* start = m_beg;
    get();
    while (is_digit(peek())) get();
    return Token(Token::Kind::Number, start, m_beg);
}

GDR_INLINE Token Lexer::slash_or_comment()
{
    char* start = m_beg;
    get();
    if (peek() == '/') {
        get();
        start = m_beg;
        while (peek() != '\0') {
            if (get() == '\n') {
                return Token(Token::Kind::Comment, start,
                    std::distance(start, m_beg) - 1);
            }
        }
        return Token(Token::Kind::Unexpected, m_beg, 1);
    }
    else {
        return Token(Token::Kind::Slash, start, 1);
    }
}