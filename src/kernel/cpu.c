#include "cpu.h"
#include "ports.h"

static struct gdt_entry gdt[5];
static struct gdt_ptr g_ptr;
static struct idt_entry idt[256];
static struct idt_ptr i_ptr;

/* Assembly wrapper to load GDT */
extern void gdt_load(struct gdt_ptr* ptr);

void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].base_low = (base & 0xFFFF);
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].base_high = (base >> 24) & 0xFF;
    gdt[i].limit_low = (limit & 0xFFFF);
    gdt[i].granularity = (limit >> 16) & 0x0F;
    gdt[i].granularity |= gran & 0xF0;
    gdt[i].access = access;
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    struct idt_entry* descriptor = &idt[vector];
    descriptor->isr_low = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs = 0x08; // Kernel Code Segment
    descriptor->ist = 0;
    descriptor->attributes = flags;
    descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

/* Generic Interrupt Handler */
__attribute__((interrupt))
void generic_handler(void* frame) {
    // If we have no way to know the vector, we can't safely EOI.
    // However, we can try to EOI both PICs just in case it's an IRQ.
    // This is a "dirty" but effective way to keep the system alive if unhandled IRQs fire.
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
    (void)frame;
}

void cpu_init() {
    /* 1. Setup GDT */
    gdt_set_entry(0, 0, 0, 0, 0);                // Null segment
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0); // Kernel Code
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xA0); // Kernel Data
    
    g_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
    g_ptr.base = (uint64_t)&gdt;
    
    // Load GDT (Directly via inline asm to avoid extra files)
    __asm__ volatile ("lgdt %0" : : "m"(g_ptr));

    /* 2. Setup IDT */
    for (int i = 0; i < 256; i++) {
        idt_set_descriptor(i, generic_handler, 0x8E);
    }

    i_ptr.limit = (sizeof(struct idt_entry) * 256) - 1;
    i_ptr.base = (uint64_t)&idt;
    
    __asm__ volatile ("lidt %0" : : "m"(i_ptr));
}

void cpu_enable_interrupts() {
    __asm__ volatile ("sti");
}
