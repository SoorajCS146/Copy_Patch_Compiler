#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory>
#include <vector>

// Forward declarations
class Expr;
class Stmt;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

// ---------------------------------------------
// Expression Base Class
// ---------------------------------------------
class Expr {
public:
    virtual ~Expr() {}
};

// Integer literal
class IntLiteral : public Expr {
public:
    int value;
    IntLiteral(int v) : value(v) {}
};

// Variable reference
class VarExpr : public Expr {
public:
    std::string name;
    VarExpr(const std::string& n) : name(n) {}
};

// Binary expression: left + right, left - right
class BinaryExpr : public Expr {
public:
    char op;           // '+' or '-'
    ExprPtr left;
    ExprPtr right;

    BinaryExpr(char o, ExprPtr l, ExprPtr r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

// ---------------------------------------------
// Statement Base Class
// ---------------------------------------------
class Stmt {
public:
    virtual ~Stmt() {}
};

// let x = expr;
class LetStmt : public Stmt {
public:
    std::string name;
    ExprPtr expr;

    LetStmt(const std::string& n, ExprPtr e)
        : name(n), expr(std::move(e)) {}
};

// print(expr);
class PrintStmt : public Stmt {
public:
    ExprPtr expr;

    PrintStmt(ExprPtr e)
        : expr(std::move(e)) {}
};

// Program = list of statements
class Program {
public:
    std::vector<StmtPtr> statements;
};

#endif // AST_HPP
