#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "stencil.hpp"
#include "codebuffer.hpp"

#include "ir.hpp"

#include "exec_stencil.hpp"
using namespace std;

string read_file_to_string(const string& path) {
    ifstream ifs(path);
    if (!ifs) return "";
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void print_ast(const Program& p) {
    cout << "Parsed AST:\n";
    for (auto& stmt : p.statements) {

        if (auto let = dynamic_pointer_cast<LetStmt>(stmt)) {
            cout << "  LetStmt: " << let->name << " = <expr>\n";
        }
        else if (auto pr = dynamic_pointer_cast<PrintStmt>(stmt)) {
            cout << "  PrintStmt(<expr>)\n";
        }
        else {
            cout << "  Unknown Stmt\n";
        }
    }
}

int main(int argc, char** argv) {
    string source;
    if (argc >= 2) {
        source = read_file_to_string(argv[1]);
    }
    if (source.empty()) {
        source = R"(
            let x = 42;
            let y = x + 5;
            print(y);
        )";
    }

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    Program program = parser.parse();

    print_ast(program);
    IRProgram ir = generate_ir(program);

    StencilLibrary lib("../stencils");
    lib.load_all();

    cout << "=== Running generated machine code (simple mode) ===\n";
    generate_and_run_with_stencils(ir, lib);

    cout << "=== IR (instr count = " << ir.size() << ") ===\n";
    for (size_t i = 0; i < ir.size(); ++i) {
        const auto &in = ir[i];
        switch (in.op) {
            case IRInstr::PUSH_CONST: cout << i << ": PUSH_CONST " << in.imm << "\n"; break;
            case IRInstr::LOAD_VAR:   cout << i << ": LOAD_VAR " << in.name << "\n"; break;
            case IRInstr::STORE_VAR:  cout << i << ": STORE_VAR " << in.name << "\n"; break;
            case IRInstr::ADD:        cout << i << ": ADD\n"; break;
            case IRInstr::SUB:        cout << i << ": SUB\n"; break;
            case IRInstr::PRINT:      cout << i << ": PRINT\n"; break;
        }
    }

    cout << "=== Running IR interpreter ===\n";
    run_ir(ir);
    // TEMP: test stencil loading
    try {
        StencilLibrary lib("../stencils");
        lib.load_all();

        // copy 'add' stencil into a buffer
        const Stencil& add = lib.get("add");
        CodeBuffer buf;
        buf.append(add.bytes.data(), add.bytes.size());

        cout << "Copied add stencil, buffer size = "
            << buf.size() << " bytes\n";
    }
    catch (const std::exception& e) {
        cerr << "Stencil error: " << e.what() << endl;
    }

    return 0;
}
