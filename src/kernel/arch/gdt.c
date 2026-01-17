#include <arch/gdt.h>

struct gdt_entry gdt[5];
struct gdt_descriptor gdtr;

extern void gdt_load(struct gdt_descriptor *gdt_ptr);

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[index].base_low    = (base & 0xFFFF);
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high   = (base >> 24) & 0xFF;

    gdt[index].limit_low   = (limit & 0xFFFF);
    gdt[index].granularity = (limit >> 16) & 0x0F;

    gdt[index].granularity |= gran & 0xF0;
    gdt[index].access      = access;
}

void gdt_init(void) {
    // 0: Null Descriptor
    gdt_set_entry(0, 0, 0, 0, 0);

    // 1: Kernel Code Segment
    // Access: Present(1) | Ring0(00) | Code/Data(1) | Exec(1) | Readable(1) | Accessed(0) -> 10011010b -> 0x9A
    // Flags: Granularity(1) | LongMode(1) | Present(0) | Avail(0) -> 1010b -> 0xA
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0);

    // 2: Kernel Data Segment
    // Access: Present(1) | Ring0(00) | Code/Data(1) | Exec(0) | Writable(1) | Accessed(0) -> 10010010b -> 0x92
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xA0);

    // 3: User Code Segment
    // Access: Present(1) | Ring3(11) | Code/Data(1) | Exec(1) | Readable(1) -> 11111010b -> 0xFA
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xA0);

    // 4: User Data Segment
    // Access: Present(1) | Ring3(11) | Code/Data(1) | Exec(0) | Writable(1) -> 11110010b -> 0xF2
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xA0);

    gdtr.size = (sizeof(struct gdt_entry) * 5) - 1;
    gdtr.offset = (uint64_t)&gdt;

    gdt_load(&gdtr);
}
