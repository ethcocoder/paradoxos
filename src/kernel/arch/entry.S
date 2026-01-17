.section .text
.global gdt_load

gdt_load:
    lgdt (%rdi)          /* Load GDT from the pointer passed in RDI */
    
    /* Reload data segment registers */
    mov $0x10, %ax       /* 0x10 is the offset for Kernel Data Segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Reload code segment register by far jump */
    pop %rdi             /* Pop return address */
    mov $0x08, %rax      /* 0x08 is the offset for Kernel Code Segment */
    push %rax            /* Push CS */
    push %rdi            /* Push return address */
    lretq                /* Far return, effectively loading CS */

.global task_switch
task_switch:
    /* RDI = &old_sp, RSI = new_sp */
    
    /* Save Callee-Saved Registers */
    push %rbx
    push %rbp
    push %r12
    push %r13
    push %r14
    push %r15
    
    /* Save current RSP to *old_sp */
    mov %rsp, (%rdi)
    
    /* Load new RSP from RSI */
    mov %rsi, %rsp
    
    /* Restore Callee-Saved Registers */
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbp
    pop %rbx
    
    ret
