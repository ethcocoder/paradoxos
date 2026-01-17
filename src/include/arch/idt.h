#ifndef _ARCH_IDT_H
#define _ARCH_IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

// Exception numbers
#define EXCEPTION_DIVIDE_BY_ZERO        0
#define EXCEPTION_DEBUG                 1
#define EXCEPTION_NMI                   2
#define EXCEPTION_BREAKPOINT            3
#define EXCEPTION_OVERFLOW              4
#define EXCEPTION_BOUND_RANGE           5
#define EXCEPTION_INVALID_OPCODE        6
#define EXCEPTION_DEVICE_NOT_AVAILABLE  7
#define EXCEPTION_DOUBLE_FAULT          8
#define EXCEPTION_INVALID_TSS           10
#define EXCEPTION_SEGMENT_NOT_PRESENT   11
#define EXCEPTION_STACK_FAULT           12
#define EXCEPTION_GENERAL_PROTECTION    13
#define EXCEPTION_PAGE_FAULT            14
#define EXCEPTION_X87_FLOATING_POINT    16
#define EXCEPTION_ALIGNMENT_CHECK       17
#define EXCEPTION_MACHINE_CHECK         18
#define EXCEPTION_SIMD_FLOATING_POINT   19
#define EXCEPTION_VIRTUALIZATION        20
#define EXCEPTION_SECURITY              30

// IRQ numbers (offset by 32)
#define IRQ_TIMER                       32
#define IRQ_KEYBOARD                    33
#define IRQ_CASCADE                     34
#define IRQ_COM2                        35
#define IRQ_COM1                        36
#define IRQ_LPT2                        37
#define IRQ_FLOPPY                      38
#define IRQ_LPT1                        39
#define IRQ_RTC                         40
#define IRQ_FREE1                       41
#define IRQ_FREE2                       42
#define IRQ_FREE3                       43
#define IRQ_PS2_MOUSE                   44
#define IRQ_FPU                         45
#define IRQ_PRIMARY_ATA                 46
#define IRQ_SECONDARY_ATA               47

struct idt_entry {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t  ist;
    uint8_t  attributes;
    uint16_t isr_mid;
    uint32_t isr_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

void idt_init(void);
void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void exception_handler(struct interrupt_frame *frame);
void irq_handler(struct interrupt_frame *frame);
extern void syscall_entry(void);

#endif
