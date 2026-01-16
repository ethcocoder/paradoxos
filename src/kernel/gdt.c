#include "gdt.h"
#include <string.h>

static struct GDT gdt;
static struct GDT_Pointer gdt_ptr;
static struct TSS tss;

void gdt_init(void) {
    // Ported from KnutOS: Standard 64-bit segments
    // Null
    gdt.null = (struct GDT_Descriptor){0,0,0,0,0,0};
    
    // Kernel Code (0x08)
    gdt.kernel_code = (struct GDT_Descriptor){0, 0, 0, 0x9A, 0xA0, 0};
    
    // Kernel Data (0x10)
    gdt.kernel_data = (struct GDT_Descriptor){0, 0, 0, 0x92, 0xA0, 0};
    
    // User Null / Data / Code
    gdt.user_null = (struct GDT_Descriptor){0,0,0,0,0,0};
    gdt.user_data = (struct GDT_Descriptor){0, 0, 0, 0x92, 0xA0, 0};
    gdt.user_code = (struct GDT_Descriptor){0, 0, 0, 0x9A, 0xA0, 0};

    // TSS
    uint64_t tss_base = (uint64_t)&tss;
    gdt.tss_low.limit_15_0 = sizeof(tss);
    gdt.tss_low.base_15_0 = tss_base & 0xFFFF;
    gdt.tss_low.base_23_16 = (tss_base >> 16) & 0xFF;
    gdt.tss_low.base_31_24 = (tss_base >> 24) & 0xFF;
    gdt.tss_low.type = 0x89;
    gdt.tss_low.limit_19_16_and_flags = 0x40;

    gdt.tss_high.limit_15_0 = (tss_base >> 32) & 0xFFFF;
    gdt.tss_high.base_15_0 = (tss_base >> 48) & 0xFFFF;

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    /* Inline Assembly for GDT loading (Windows compatible) */
    asm volatile (
        "lgdt %0\n\t"
        "pushq $0x08\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        : : "m"(gdt_ptr) : "rax", "memory"
    );

    // Load TSS
    asm volatile ("ltr %%ax" : : "a"(0x30));
}
