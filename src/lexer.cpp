#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

using namespace std;

static unordered_map<string, TokenType> keywords = {
    {"let",   TokenType::LET},
    {"print", TokenType::PRINT}
};

string token_type_to_string(TokenType t) {
    switch (t) {
        case TokenType::LET: return "LET";
        case TokenType::PRINT: return "PRINT";
        case TokenType::IDENT: return "IDENT";
        case TokenType::INT_LIT: return "INT_LIT";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::SEMI: return "SEMI";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::END: return "END";
        default: return "UNKNOWN";
    }
}

Lexer::Lexer(const string& input_)
    : input(input_), pos(0), length(input_.size()) {}

char Lexer::peek() const {
    if (pos >= length) return '\0';
    return input[pos];
}

char Lexer::get() {
    if (pos >= length) return '\0';
    return input[pos++];
}

void Lexer::skip_whitespace() {
    while (true) {
        char c = peek();
        if (c == '\0') return;
        if (isspace(static_cast<unsigned char>(c))) { get(); continue; }
        // skip comments starting with // to end of line
        if (c == '/' && pos + 1 < length && input[pos+1] == '/') {
            // consume until newline or end
            get(); get(); // consume //
            while (peek() != '\0' && peek() != '\n') get();
            continue;
        }
        break;
    }
}

bool Lexer::is_alpha(char c) const {
    return (std::isalpha(static_cast<unsigned char>(c)) || c == '_');
}

bool Lexer::is_alnum(char c) const {
    return (std::isalnum(static_cast<unsigned char>(c)) || c == '_');
}

Token Lexer::read_number() {
    string s;
    while (isdigit(static_cast<unsigned char>(peek()))) {
        s.push_back(get());
    }
    // convert to int64
    int64_t val = 0;
    try {
        val = stoll(s);
    } catch (...) {
        val = 0;
    }
    return Token(TokenType::INT_LIT, s, val);
}

Token Lexer::read_ident_or_keyword() {
    string s;
    while (is_alnum(peek())) s.push_back(get());
    auto it = keywords.find(s);
    if (it != keywords.end()) {
        return Token(it->second, s, 0);
    }
    return Token(TokenType::IDENT, s, 0);
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;
    while (true) {
        skip_whitespace();
        char c = peek();
        if (c == '\0') {
            tokens.emplace_back(TokenType::END, "", 0);
            break;
        }

        if (is_alpha(c)) {
            tokens.push_back(read_ident_or_keyword());
            continue;
        }

        if (isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(read_number());
            continue;
        }

        // single-char tokens
        switch (c) {
            case '=': get(); tokens.emplace_back(TokenType::ASSIGN, "=", 0); break;
            case '+': get(); tokens.emplace_back(TokenType::PLUS, "+", 0); break;
            case '-': get(); tokens.emplace_back(TokenType::MINUS, "-", 0); break;
            case ';': get(); tokens.emplace_back(TokenType::SEMI, ";", 0); break;
            case '(': get(); tokens.emplace_back(TokenType::LPAREN, "(", 0); break;
            case ')': get(); tokens.emplace_back(TokenType::RPAREN, ")", 0); break;
            default:
                // unknown char: return as UNKNOWN token with lexeme
                {
                    string s;
                    s.push_back(get());
                    tokens.emplace_back(TokenType::UNKNOWN, s, 0);
                }
                break;
        }
    }
    return tokens;
}
