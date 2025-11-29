#include "ir.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdexcept>

using namespace std;

// Simple stack-machine interpreter for IRProgram.
// Prints output to stdout for PRINT instructions.
void run_ir(const IRProgram &prog) {
    vector<int64_t> stk;
    unordered_map<string, int64_t> env;

    auto push = [&](int64_t v){ stk.push_back(v); };
    auto pop = [&]()->int64_t {
        if (stk.empty()) throw runtime_error("IR runtime: pop from empty stack");
        int64_t v = stk.back(); stk.pop_back(); return v;
    };

    for (size_t ip = 0; ip < prog.size(); ++ip) {
        const IRInstr &ins = prog[ip];
        switch (ins.op) {
            case IRInstr::PUSH_CONST:
                push(ins.imm);
                break;

            case IRInstr::LOAD_VAR: {
                auto it = env.find(ins.name);
                if (it == env.end()) {
                    // undefined variables default to 0 (or throw)
                    push(0);
                } else {
                    push(it->second);
                }
                break;
            }

            case IRInstr::STORE_VAR: {
                int64_t v = pop();
                env[ins.name] = v;
                break;
            }

            case IRInstr::ADD: {
                int64_t b = pop();
                int64_t a = pop();
                push(a + b);
                break;
            }

            case IRInstr::SUB: {
                int64_t b = pop();
                int64_t a = pop();
                push(a - b);
                break;
            }

            case IRInstr::PRINT: {
                int64_t v = pop();
                cout << v << "\n";
                break;
            }

            default:
                throw runtime_error("IR runtime: unknown opcode");
        }
    }
}
