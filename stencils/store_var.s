    .globl store_var
store_var:
    mov %rax, -8(%rbp)      # offset will be patched
    ret
