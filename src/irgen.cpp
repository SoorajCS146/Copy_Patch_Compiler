#include "ir.hpp"
#include "ast.hpp"
#include <memory>
#include <stdexcept>

using namespace std;

// Forward declarations
static void genExpr(const ExprPtr &expr, IRProgram &out);
static void genStmt(const StmtPtr &stmt, IRProgram &out);

// Top-level: generate IR for entire Program
IRProgram generate_ir(const Program &p) {
    IRProgram out;
    for (const auto &s : p.statements) genStmt(s, out);
    return out;
}

// Statements
static void genStmt(const StmtPtr &stmt, IRProgram &out) {
    if (auto let = dynamic_pointer_cast<LetStmt>(stmt)) {
        // evaluate RHS -> leaves value on stack
        genExpr(let->expr, out);
        // store into variable (pop)
        out.emplace_back(IRInstr::STORE_VAR, let->name);
    }
    else if (auto pr = dynamic_pointer_cast<PrintStmt>(stmt)) {
        genExpr(pr->expr, out);
        out.emplace_back(IRInstr::PRINT);
    }
    else {
        throw runtime_error("Unknown statement type in IR generation");
    }
}

// Expressions (stack-based)
static void genExpr(const ExprPtr &expr, IRProgram &out) {
    if (auto il = dynamic_pointer_cast<IntLiteral>(expr)) {
        out.emplace_back((int64_t)il->value);
        return;
    }
    if (auto ve = dynamic_pointer_cast<VarExpr>(expr)) {
        out.emplace_back(IRInstr::LOAD_VAR, ve->name);
        return;
    }
    if (auto be = dynamic_pointer_cast<BinaryExpr>(expr)) {
        // stack machine: evaluate left then right then op
        genExpr(be->left, out);
        genExpr(be->right, out);
        if (be->op == '+') out.emplace_back(IRInstr::ADD);
        else if (be->op == '-') out.emplace_back(IRInstr::SUB);
        else throw runtime_error("Unsupported binary operator in IR: " + string(1, be->op));
        return;
    }

    throw runtime_error("Unknown expression type in IR generation");
}
