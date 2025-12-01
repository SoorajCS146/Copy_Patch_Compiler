#include <iostream>

#include <unistd.h>     // sysconf, _SC_PAGESIZE
#include <sys/mman.h>     // mmap, PROT_*, MAP_*
#include <cstring>     // memcpy

#include <fstream>
#include <sstream>

#include <filesystem>	// For dynamic path detection.

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "stencil.hpp"
#include "codebuffer.hpp"

#include "ir.hpp"

#include "exec_stencil.hpp"
using namespace std;

namespace fs = std::filesystem;  // For dynamic path detection.

void* g_print_int_fn = nullptr;   // global used by codegen_simple.cpp

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

    fs::path stencil_path;

    if(fs::exists("stencils")) {
	    stencil_path = "stencils";
    }
    else if(fs::exists("../stencils")) {
	    stencil_path = "../stencils";
    }
    else {
	    cerr << "Stencil directory not found\n";
	    return 1;
    }

    // StencilLibrary lib("../stencils");
    // OLDER Platform-based version (redundant now).
    // ifdef _WIN32
    //	StencilLibrary lib("stencils");
    // else
    //  StencilLibrary lib("stencils");
    // endif

    StencilLibrary lib(stencil_path.string() );		// For dynamic path detection.
    lib.load_all();

    cout << "=== Running generated machine code (simple mode) ===\n";

    // *** Loading print-stencil.
    const Stencil& pst = lib.get("print_int");  
        
    // allocating exec memory for print stencil.
    size_t sz = pst.bytes.size();
    size_t pagesz = sysconf(_SC_PAGESIZE);
    size_t allocsz = ( (sz + pagesz - 1) / pagesz) * pagesz;

    void *print_fn_mem = mmap(nullptr, allocsz,
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (print_fn_mem == MAP_FAILED) {
        perror("mmap print");
        exit(1);
    }

    memcpy(print_fn_mem, pst.bytes.data(), pst.bytes.size() );

    g_print_int_fn = print_fn_mem;
    std::cerr << "[DEBUG] g_print_int_fn = " << g_print_int_fn << "\n";


    generate_and_run_with_stencils(ir, lib);

    // cleanup print stencil
    munmap(print_fn_mem, allocsz);
    g_print_int_fn = nullptr;

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
	    // Commenting +3 upon BB's suggestion.
//        StencilLibrary lib("stencils");
//        // StencilLibrary lib("../stencils"); => changing as per BB.
//        lib.load_all();

        // Reusing the already loaded library.

        // add-stencil.
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
