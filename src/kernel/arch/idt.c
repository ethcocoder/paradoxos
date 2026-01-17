#include <arch/idt.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <mm/vmm.h>
#include <kernel.h>

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtr;

extern void *isr_stub_table[];

// Exception names for debugging
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
    uint64_t addr = (uint64_t)isr;
    
    idt[vector].isr_low    = addr & 0xFFFF;
    idt[vector].kernel_cs  = 0x08; // Kernel Code Segment
    idt[vector].ist        = 0;
    idt[vector].attributes = flags;
    idt[vector].isr_mid    = (addr >> 16) & 0xFFFF;
    idt[vector].isr_high   = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved   = 0;
}

void display_panic_screen(const char *exception_name, struct interrupt_frame *frame) {
    // Clear screen with dark gray background
    fb_clear(0x2D2D30);
    
    // Display panic header
    fb_draw_rect(0, 0, 1920, 80, 0x1E1E1E);
    
    // TODO: Implement text rendering for panic screen
    // For now, just halt the system
    printk("\n=== PARADOX OS KERNEL PANIC ===\n");
    printk("Exception: %s\n", exception_name);
    printk("Error Code: 0x%lx\n", frame->err_code);
    printk("RIP: 0x%lx\n", frame->rip);
    printk("RSP: 0x%lx\n", frame->rsp);
    printk("RAX: 0x%lx  RBX: 0x%lx\n", frame->rax, frame->rbx);
    printk("RCX: 0x%lx  RDX: 0x%lx\n", frame->rcx, frame->rdx);
    printk("RSI: 0x%lx  RDI: 0x%lx\n", frame->rsi, frame->rdi);
    printk("RBP: 0x%lx  RFLAGS: 0x%lx\n", frame->rbp, frame->rflags);
    printk("\nSystem Halted. Please restart.\n");
}

void exception_handler(struct interrupt_frame *frame) {
    const char *exception_name = "Unknown Exception";
    
    if (frame->int_no < 32) {
        exception_name = exception_messages[frame->int_no];
    }
    
    // Special handling for specific exceptions
    switch (frame->int_no) {
        case EXCEPTION_DIVIDE_BY_ZERO:
            printk("Division by zero at RIP: 0x%lx\n", frame->rip);
            break;
            
        case EXCEPTION_PAGE_FAULT: {
            uint64_t faulting_address;
            __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));
            
            // Try to handle the page fault through VMM
            handle_page_fault(faulting_address, frame->err_code);
            
            // If we reach here, the page fault wasn't handled
            printk("Unhandled page fault at address: 0x%lx\n", faulting_address);
            printk("Error code: 0x%lx\n", frame->err_code);
            if (frame->err_code & 0x1) printk("  - Page protection violation\n");
            else printk("  - Page not present\n");
            if (frame->err_code & 0x2) printk("  - Write operation\n");
            else printk("  - Read operation\n");
            if (frame->err_code & 0x4) printk("  - User mode\n");
            else printk("  - Kernel mode\n");
            break;
        }
        
        case EXCEPTION_GENERAL_PROTECTION:
            printk("General Protection Fault - Segment: 0x%lx\n", frame->err_code);
            break;
            
        case EXCEPTION_DOUBLE_FAULT:
            printk("Double Fault - This is critical!\n");
            break;
    }
    
    display_panic_screen(exception_name, frame);
    
    // Disable interrupts and halt
    __asm__ volatile("cli");
    while(1) {
        __asm__ volatile("hlt");
    }
}

void irq_handler(struct interrupt_frame *frame) {
    // Handle IRQs
    switch (frame->int_no) {
        case IRQ_TIMER:
            // Timer interrupt - could be used for scheduling
            break;
            
        case IRQ_KEYBOARD:
            // Call keyboard handler
            keyboard_handler_c();
            break;
            
        case IRQ_PS2_MOUSE:
            // Call mouse handler
            mouse_handler_c();
            break;
            
        default:
            printk("Unhandled IRQ: %lu\n", frame->int_no);
            break;
    }
    
    // Send EOI to PIC
    if (frame->int_no >= 40) {
        // Secondary PIC
        __asm__ volatile("outb %%al, %0" : : "Nd"(0xA0), "a"(0x20));
    }
    // Primary PIC
    __asm__ volatile("outb %%al, %0" : : "Nd"(0x20), "a"(0x20));
}

void idt_init(void) {
    idtr.base = (uint64_t)&idt;
    idtr.limit = (uint16_t)(sizeof(struct idt_entry) * IDT_ENTRIES - 1);

    // Clear IDT
    memset(&idt, 0, sizeof(idt));

    // Install exception handlers (0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_descriptor(i, isr_stub_table[i], 0x8E); // Interrupt Gate
    }
    
    // Install IRQ handlers (32-47)
    for (int i = 32; i < 48; i++) {
        if (isr_stub_table[i] != 0) {
            idt_set_descriptor(i, isr_stub_table[i], 0x8E);
        }
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // Load IDT
    
    printk("[IDT] Interrupt Descriptor Table initialized with %d entries\n", IDT_ENTRIES);
}
