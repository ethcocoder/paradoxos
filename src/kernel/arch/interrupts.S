.section .text
.macro ISR_NOERRCODE nr
    .global isr\nr
    isr\nr:
        push $0     /* Push dummy error code */
        push $\nr   /* Push interrupt number */
        jmp isr_common_stub
.endm

.macro ISR_ERRCODE nr
    .global isr\nr
    isr\nr:
        push $\nr   /* Push interrupt number */
        jmp isr_common_stub
.endm

/* Define first 32 exceptions */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31

/* IRQ handlers (32-47) */
.macro IRQ nr
    .global isr\nr
    isr\nr:
        push $0     /* Push dummy error code */
        push $\nr   /* Push interrupt number */
        jmp irq_common_stub
.endm

IRQ 32  /* Timer */
IRQ 33  /* Keyboard */
IRQ 34  /* Cascade */
IRQ 35  /* COM2 */
IRQ 36  /* COM1 */
IRQ 37  /* LPT2 */
IRQ 38  /* Floppy */
IRQ 39  /* LPT1 */
IRQ 40  /* RTC */
IRQ 41  /* Free */
IRQ 42  /* Free */
IRQ 43  /* Free */
IRQ 44  /* PS/2 Mouse */
IRQ 45  /* FPU */
IRQ 46  /* Primary ATA */
IRQ 47  /* Secondary ATA */

/* Table of pointers to ISRs */
.section .data
.global isr_stub_table
isr_stub_table:
    .quad isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
    .quad isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
    .quad isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
    .quad isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    .quad isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39
    .quad isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47

.section .text
.extern exception_handler
.extern irq_handler

/* Common exception handler */
isr_common_stub:
    /* Push all registers */
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    /* Call C exception handler with stack frame */
    mov %rsp, %rdi
    call exception_handler
    
    /* Restore registers */
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    /* Cleanup error code and ISR number */
    add $16, %rsp
    iretq

/* Common IRQ handler */
irq_common_stub:
    /* Push all registers */
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    /* Call C IRQ handler with stack frame */
    mov %rsp, %rdi
    call irq_handler
    
    /* Restore registers */
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    /* Cleanup error code and ISR number */
    add $16, %rsp
    iretq
