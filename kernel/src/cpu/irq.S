.extern exception_handler
.global _default_interrupt_handlers

.macro isr_stub num error_code=0
    .section .text
    .align 8
    .extern isr_stub_\num
    isr_stub_\num:

        // If no need for error code, push 0
        .if \error_code == 0
        pushq   $0
        .endif

        // Push exception number
        pushq   $\num

        // Save registers
        pushq   %rbp
        pushq   %rdi
        pushq   %rsi
        pushq   %rdx
        pushq   %rcx
        pushq   %rbx
        pushq   %rax
        pushq   %r8
        pushq   %r9
        pushq   %r10
        pushq   %r11
        pushq   %r12
        pushq   %r13
        pushq   %r14
        pushq   %r15

        // Pass stack pointer as first argument
        mov %rsp, %rdi

        // Call common exception handler
        call exception_handler

        // Use the returned stack pointer
        mov %rax, %rsp

        // Restore registers
        popq    %r15
        popq    %r14
        popq    %r13
        popq    %r12
        popq    %r11
        popq    %r10
        popq    %r9
        popq    %r8
        popq    %rax
        popq    %rbx
        popq    %rcx
        popq    %rdx
        popq    %rsi
        popq    %rdi
        popq    %rbp

        // discard error code & exception number
        addq    $16, %rsp

        iretq

    .section .data
    .quad isr_stub_\num
.endm

.section .data
_default_interrupt_handlers:
isr_stub 0
isr_stub 1
isr_stub 2
isr_stub 3
isr_stub 4
isr_stub 5
isr_stub 6
isr_stub 7
isr_stub 8 error_code=1
isr_stub 9
isr_stub 10 error_code=1
isr_stub 11 error_code=1
isr_stub 12 error_code=1
isr_stub 13 error_code=1
isr_stub 14 error_code=1
isr_stub 15
isr_stub 16
isr_stub 17 error_code=1
isr_stub 18
isr_stub 19
isr_stub 20
isr_stub 21 error_code=1
isr_stub 22
isr_stub 23
isr_stub 24
isr_stub 25
isr_stub 26
isr_stub 27
isr_stub 28
isr_stub 29 error_code=1
isr_stub 30 error_code=1
isr_stub 31