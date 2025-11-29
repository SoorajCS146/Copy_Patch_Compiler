    .globl load_var
load_var:
    mov -8(%rbp), %rax      # offset will be patched
    ret
