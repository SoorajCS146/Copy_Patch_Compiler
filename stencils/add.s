    .globl add
add:
    mov %rdi, %rax     # rax = arg1
    add %rsi, %rax     # rax += arg2
    ret
