#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct IDT_Descriptor {
    uint16_t offset_15_0;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_and_attributes;
    uint16_t offset_31_16;
    uint32_t offset_63_32;
    uint32_t zero;
} __attribute__((packed));

struct IDT_Pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init(void);

#endif
