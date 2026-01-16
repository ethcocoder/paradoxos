#include "idt.h"
#include "ports.h"
#include "serial.h"

static struct IDT_Descriptor idt[256];
static struct IDT_Pointer idt_ptr;

/* Common ISR handler */
void isr_handler(void) {
    serial_send_string("[PARADOX] Interrupt Triggered!\n");
}

/* Generic Stub (GCC Interrupt Attribute) */
__attribute__((interrupt))
void generic_handler(void* frame) {
    (void)frame;
    isr_handler();
}

void idt_set_descriptor(uint8_t vector, void* handler, uint8_t flags) {
    uint64_t addr = (uintptr_t)handler;
    idt[vector].offset_15_0 = addr & 0xFFFF;
    idt[vector].selector = 0x08; // Kernel Code
    idt[vector].ist = 0;
    idt[vector].type_and_attributes = flags;
    idt[vector].offset_31_16 = (addr >> 16) & 0xFFFF;
    idt[vector].offset_63_32 = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

void idt_init(void) {
    // Fill with generic handler
    for (int i = 0; i < 256; i++) {
        idt_set_descriptor(i, generic_handler, 0x8E);
    }
    
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uintptr_t)&idt;

    asm volatile ("lidt %0" : : "m"(idt_ptr));
}
