#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"
#include <vector>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    Program parse();

private:
    const std::vector<Token>& tokens;
    size_t pos;

    const Token& peek() const;
    const Token& get();
    bool match(TokenType t);

    // parsing functions
    StmtPtr parse_statement();
    StmtPtr parse_let();
    StmtPtr parse_print();

    ExprPtr parse_expression();
    ExprPtr parse_term();
    ExprPtr parse_primary();

    void expect(TokenType t, const std::string& msg);
};

#endif // PARSER_HPP
