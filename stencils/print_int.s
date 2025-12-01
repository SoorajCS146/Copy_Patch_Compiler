    .globl print_int
print_int:
    # RAX contains the integer

    push %rbp
    mov %rsp, %rbp

    # allocate small buffer (32 bytes)
    sub $32, %rsp

    lea -32(%rbp), %rsi     # buffer
    mov %rax, %rdi          # number

    # convert integer to string
    mov $0, %rcx            # index

convert_loop:
    xor %rdx, %rdx
    mov $10, %rbx
    div %rbx                # rax = rax/10, rdx = remainder

    add $48, %rdx           # convert digit to ASCII
    mov %dl, (%rsi, %rcx, 1)
    inc %rcx

    cmp $0, %rax
    jne convert_loop

    # reverse string
    # save length (rcx == digit count) into r11, then prepare for reverse
    mov %rcx, %r11
    mov $0, %rdx
    dec %rcx
rev_loop:
    cmp %rdx, %rcx
    jle done_rev
    mov (%rsi,%rdx), %bl
    mov (%rsi,%rcx), %bh
    mov %bh, (%rsi,%rdx)
    mov %bl, (%rsi,%rcx)
    inc %rdx
    dec %rcx
    jmp rev_loop

done_rev:
    # write syscall
    mov $1, %rax        # SYS_write
    mov $1, %rdi        # stdout
    mov %rsi, %rsi      # buffer (no-op)
    mov %r11, %rdx      # length (use saved digit count)
    syscall

    # print newline
    mov $1, %rax
    mov $1, %rdi
    # use movabs with a placeholder immediate; loader will patch this imm64
    movabs $0, %rsi
    mov $1, %rdx
    syscall

    leave
    ret

.section .rodata
newline:
    .ascii "\n"
