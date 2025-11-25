#ifndef IR_HPP
#define IR_HPP

#include <cstdint>
#include <string>
#include <vector>

struct IRInstr {
    enum Op : uint8_t {
        PUSH_CONST,   // imm -> push constant
        LOAD_VAR,     // name -> push var value
        STORE_VAR,    // name -> pop into var
        ADD,          // a+b
        SUB,          // a-b
        PRINT         // pop and print
    } op;

    int64_t imm;
    std::string name;

    IRInstr(Op o) : op(o), imm(0) {}
    IRInstr(int64_t c) : op(PUSH_CONST), imm(c) {}
    IRInstr(Op o, const std::string &n) : op(o), imm(0), name(n) {}
};

using IRProgram = std::vector<IRInstr>;

// 🔥 You MUST add these function declarations:
IRProgram generate_ir(const class Program &p);
void run_ir(const IRProgram &prog);

#endif // IR_HPP
