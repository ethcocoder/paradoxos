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
    
    # System call arguments are in:
    # RAX = system call number
    # RDI = arg1
    # RSI = arg2  
    # RDX = arg3
    # R10 = arg4 (RCX is used by SYSCALL instruction)
    # R8  = arg5
    # R9  = arg6
    
    # Move arguments to match C calling convention
    mov %rax, %rdi  # syscall_num
    mov %rdi, %rsi  # arg1 (original RDI)
    mov %rsi, %rdx  # arg2 (original RSI)
    mov %rdx, %rcx  # arg3 (original RDX)
    mov %r10, %r8   # arg4
    mov %r8, %r9    # arg5
    # arg6 is already in the right place on stack
    
    # Call C handler
    call syscall_handler
    
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