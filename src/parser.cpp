#include "parser.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

Parser::Parser(const vector<Token>& toks)
    : tokens(toks), pos(0) {}

const Token& Parser::peek() const {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

const Token& Parser::get() {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos++];
}

bool Parser::match(TokenType t) {
    if (peek().type == t) {
        get();
        return true;
    }
    return false;
}

void Parser::expect(TokenType t, const string& msg) {
    if (!match(t)) {
        throw runtime_error("Parse error: expected " + msg +
            " but found '" + peek().lexeme + "'");
    }
}

Program Parser::parse() {
    Program p;
    while (peek().type != TokenType::END) {
        p.statements.push_back(parse_statement());
    }
    return p;
}

StmtPtr Parser::parse_statement() {
    if (peek().type == TokenType::LET) return parse_let();
    if (peek().type == TokenType::PRINT) return parse_print();

    throw runtime_error("Unexpected token in statement: " + peek().lexeme);
}

StmtPtr Parser::parse_let() {
    get(); // consume 'let'
    string name = peek().lexeme;
    expect(TokenType::IDENT, "identifier");

    expect(TokenType::ASSIGN, "=");

    ExprPtr expr = parse_expression();

    expect(TokenType::SEMI, ";");

    return make_shared<LetStmt>(name, expr);
}

StmtPtr Parser::parse_print() {
    get(); // consume 'print'

    expect(TokenType::LPAREN, "(");
    ExprPtr expr = parse_expression();
    expect(TokenType::RPAREN, ")");

    expect(TokenType::SEMI, ";");

    return make_shared<PrintStmt>(expr);
}

// ---------------------- Expressions ----------------------

ExprPtr Parser::parse_expression() {
    ExprPtr left = parse_term();

    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
        char op = (peek().type == TokenType::PLUS) ? '+' : '-';
        get(); // consume operator
        ExprPtr right = parse_term();
        left = make_shared<BinaryExpr>(op, left, right);
    }

    return left;
}

ExprPtr Parser::parse_term() {
    return parse_primary();
}

ExprPtr Parser::parse_primary() {
    const Token& t = peek();

    if (t.type == TokenType::INT_LIT) {
        get();
        return make_shared<IntLiteral>(t.int_value);
    }

    if (t.type == TokenType::IDENT) {
        string name = t.lexeme;
        get();
        return make_shared<VarExpr>(name);
    }

    if (t.type == TokenType::LPAREN) {
        get(); // consume '('
        ExprPtr inside = parse_expression();
        expect(TokenType::RPAREN, ")");
        return inside;
    }

    throw runtime_error("Unexpected token in expression: " + t.lexeme);
}
