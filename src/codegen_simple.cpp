// src/codegen_simple.cpp
#include "ir.hpp"
#include "codebuffer.hpp"
#include "stencil.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <fstream>

using namespace std;

extern "C" void* g_print_int_fn;

// Small helper: emit little-known instruction bytes for common ops.
static void emit_prologue(CodeBuffer &cb) {
    // push rbp
    cb.push(0x55);
    // mov rbp, rsp
    cb.append((const uint8_t*)"\x48\x89\xe5", 3);
    // sub rsp, 128  => 48 81 ec 80 00 00 00
    cb.append((const uint8_t*)"\x48\x81\xec", 3);
    cb.write32_le(128);
}

static void emit_epilogue(CodeBuffer &cb) {
    // leave; ret
    cb.append((const uint8_t*)"\xc9\xc3", 2);
}

// movabs rax, imm64    => 48 b8 <imm64>
static void emit_movabs_rax_imm64(CodeBuffer &cb, int64_t imm) {
    cb.append((const uint8_t*)"\x48\xb8", 2);
    cb.write64_le(imm);
}

// movabs rbx, imm64    => 48 bc <imm64>
static void emit_movabs_rbx_imm64(CodeBuffer &cb, int64_t imm) {
    cb.append((const uint8_t*)"\x48\xBC", 2);
    cb.write64_le(imm);
}

// push rax
static void emit_push_rax(CodeBuffer &cb) { cb.push(0x50); }
// pop rax
static void emit_pop_rax(CodeBuffer &cb) { cb.push(0x58); }
// pop rbx
static void emit_pop_rbx(CodeBuffer &cb) { cb.push(0x5b); }

// mov rax, qword ptr [rbp - disp8]  => 48 8b 45 <disp8>
static void emit_mov_rax_from_rbp_disp8(CodeBuffer &cb, int8_t disp8) {
    cb.append((const uint8_t*)"\x48\x8b\x45", 3);
    cb.push((uint8_t)disp8);
}

// mov qword ptr [rbp - disp8], rax => 48 89 45 <disp8>
static void emit_mov_rbp_disp8_from_rax(CodeBuffer &cb, int8_t disp8) {
    cb.append((const uint8_t*)"\x48\x89\x45", 3); // three opcode bytes
    cb.push((uint8_t)disp8);
}


// add rax, rbx  => 48 01 d8
static void emit_add_rax_rbx(CodeBuffer &cb) {
    cb.append((const uint8_t*)"\x48\x01\xd8", 3);
}

// sub rax, rbx => 48 29 d8
static void emit_sub_rax_rbx(CodeBuffer &cb) {
    cb.append((const uint8_t*)"\x48\x29\xd8", 3);
}

// helper to append raw stencil bytes (already loaded as .o raw bytes)
static void append_stencil_bytes(CodeBuffer &cb, const Stencil &s) {
    if (!s.bytes.empty()) cb.append_vec(s.bytes);
}

// The main function: generate code for IR program and execute it
void generate_and_run_with_stencils(const IRProgram &ir, StencilLibrary &lib) {
    CodeBuffer cb;

    // simple variable layout: assign offsets (8,16,24...) for variables seen in STORE_VAR
    unordered_map<string, int> var_slot; // maps var -> offset (positive)
    int next_slot = 8;

    // prologue
    emit_prologue(cb);

    // Walk IR and emit code
    for (size_t ip = 0; ip < ir.size(); ++ip) {
        const IRInstr &ins = ir[ip];
        switch (ins.op) {
            case IRInstr::PUSH_CONST: {
                // movabs rax, imm64; push rax
                emit_movabs_rax_imm64(cb, ins.imm);
                emit_push_rax(cb);
                break;
            }

            case IRInstr::LOAD_VAR: {
                auto it = var_slot.find(ins.name);
                int slot = 0;
                if (it == var_slot.end()) {
                    // undefined variable -> default 0
                    emit_movabs_rax_imm64(cb, 0);
                    emit_push_rax(cb);
                } else {
                    slot = it->second;
                    // mov rax, [rbp - slot]; push rax
                    
                    // slot is a positive byte offset (8,16,..). Instructions use [rbp - slot],
                    // so pass (int8_t)(-slot) to encode the signed disp8 correctly.

                    emit_mov_rax_from_rbp_disp8(cb, (int8_t)(-slot));   // When reading a slot, pass a negative disp8.  ## BB-edit
                    // emit_mov_rax_from_rbp_disp8(cb, (int8_t)slot);
                    emit_push_rax(cb);
                }
                break;
            }

            case IRInstr::STORE_VAR: {
                // pop rax ; mov [rbp - slot], rax
                emit_pop_rax(cb);
                int slot;
                auto it = var_slot.find(ins.name);
                if (it == var_slot.end()) {
                    slot = next_slot;
                    var_slot[ins.name] = slot;
                    next_slot += 8;
                } else {
                    slot = it->second;
                }
                
                // slot is a positive byte offset (8,16,..). Instructions use [rbp - slot],
                // so pass (int8_t)(-slot) to encode the signed disp8 correctly.
                
                emit_mov_rbp_disp8_from_rax(cb, (int8_t)(-slot));   // When writing a slot, pass a negative disp8.  ## BB-edit
                // emit_mov_rbp_disp8_from_rax(cb, (int8_t)slot);
                break;
            }

            case IRInstr::ADD: {
                // stack: ... left, right
                // pop right -> rbx ; pop left -> rax ; add rax, rbx ; push rax
                emit_pop_rbx(cb);
                emit_pop_rax(cb);
                emit_add_rax_rbx(cb);
                emit_push_rax(cb);
                break;
            }

            case IRInstr::SUB: {
                emit_pop_rbx(cb);
                emit_pop_rax(cb);
                emit_sub_rax_rbx(cb);
                emit_push_rax(cb);
                break;
            }

            case IRInstr::PRINT: {
                // pop value into rax (the stencil expects value in RAX)
                emit_pop_rax(cb);   // value in RAX.

                // load print stencil address into RBX (preserve RAX)
                emit_movabs_rbx_imm64(cb, (int64_t)g_print_int_fn);

                // call rbx -> opcode: FF D3
                cb.append( (const uint8_t*)"\xFF\xD3", 2);

                // v3.
                // // move RDI, RAX  (argument 1 = RDI)
                // // opcode: 48 89 C7
                // cb.append((const uint8_t*)"\x48\x89\xC7", 3);

                // // load print fn address into RAX (movabs rax, imm64)
                // emit_movabs_rax_imm64(cb, (int64_t)g_print_int_fn);

                // // call rax -> opcode: FF D0
                // cb.append( (const uint8_t*)"\xFF\xD0", 2);

                // v2.
                // cb.push(0xE8);

                // uint64_t next_instr = cb.current_ip() + 4;
                // uint64_t target = (uint64_t)g_print_int_fn;

                // int32_t rel = (int32_t)(target - next_instr);
                // cb.write32_le(rel);


                // // append the print_int stencil bytes (assumes it consumes RAX)
                // const Stencil &pst = lib.get("print_int");
                // append_stencil_bytes(cb, pst);
                break;
            }

            default:
                throw runtime_error("Unknown IR opcode in codegen");
        }
    }

    // epilogue
    emit_epilogue(cb);

    // ---- allocate executable memory and run ----
    size_t codesz = cb.size();
    // round up to page size
    size_t pagesz = sysconf(_SC_PAGESIZE);
    size_t allocsz = ((codesz + pagesz - 1) / pagesz) * pagesz;

    std::ofstream dbg("/tmp/jit.bin", std::ios::binary);
    dbg.write((char*)cb.data(), cb.size());
    dbg.close();
    // hexdump -C /tmp/jit.bin | sed -n '0,40p'

    void *mem = mmap(nullptr, allocsz,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        throw runtime_error("mmap failed");
    }

    // copy code into mem
    memcpy(mem, cb.data(), codesz);

    // flush instruction cache if required (generally not necessary on x86)
    typedef int64_t (*fn_t)();
    fn_t fn = (fn_t)mem;

    // call generated function and capture return value (RAX)
    int64_t ret = fn();
    std::cout << ">>> generated program returned: " << ret << std::endl;

    // cleanup
    munmap(mem, allocsz);

}
