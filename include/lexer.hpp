#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <cstdint>

enum class TokenType {
    // Keywords
    LET,
    PRINT,

    // Primary tokens
    IDENT,      // variable names
    INT_LIT,    // integer literals

    // Operators / punctuation
    ASSIGN,     // =
    PLUS,       // +
    MINUS,      // -
    SEMI,       // ;
    LPAREN,     // (
    RPAREN,     // )

    // End / error
    END,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string lexeme; // actual text for IDENT/INT_LIT etc
    int64_t int_value;  // meaningful when type == INT_LIT

    Token(TokenType t = TokenType::UNKNOWN, const std::string& s = "", int64_t v = 0)
        : type(t), lexeme(s), int_value(v) {}
};

// Convert token type to printable string
std::string token_type_to_string(TokenType t);

// Lexer class: given an input string, returns tokens
class Lexer {
public:
    explicit Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    const std::string input;
    size_t pos;
    size_t length;

    char peek() const;
    char get();
    void skip_whitespace();
    bool is_alpha(char c) const;
    bool is_alnum(char c) const;

    Token read_number();
    Token read_ident_or_keyword();
};

#endif // LEXER_HPP
