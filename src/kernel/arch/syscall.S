.section .text

# System call entry point (INT 0x80)
.global syscall_entry
syscall_entry:
    # Save all registers
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
    
    # Stack after pushes:
    # RSP + 0: R15
    # RSP + 8: R14
    # RSP + 16: R13
    # RSP + 24: R12
    # RSP + 32: R11
    # RSP + 40: R10
    # RSP + 48: R9
    # RSP + 56: R8
    # RSP + 64: RBP
    # RSP + 72: RDI
    # RSP + 80: RSI
    # RSP + 88: RDX
    # RSP + 96: RCX
    # RSP + 104: RBX
    # RSP + 112: RAX

    # Move arguments to match C calling convention for syscall_handler:
    # int64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, 
    #                        uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
    # RDI = syscall_num (RAX)
    # RSI = arg1 (RDI)
    # RDX = arg2 (RSI)
    # RCX = arg3 (RDX)
    # R8  = arg4 (R10)
    # R9  = arg5 (R8)
    # [stack] = arg6 (R9)

    mov 112(%rsp), %rdi # syscall_num (RAX)
    mov 72(%rsp), %rsi  # arg1 (RDI)
    mov 80(%rsp), %rdx  # arg2 (RSI)
    mov 88(%rsp), %rcx  # arg3 (RDX)
    mov 40(%rsp), %r8   # arg4 (R10)
    mov 56(%rsp), %r9   # arg5 (R8)
    
    # Push arg6 (R9) to stack as the 7th argument for C function
    push 48(%rsp)

    call syscall_handler
    
    # Clean up the 7th argument from stack
    add $8, %rsp
    
    # Return value is in RAX, leave it there
    
    # Restore registers (except RAX which contains return value)
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
    add $8, %rsp    # Skip saved RAX
    
    iretq
