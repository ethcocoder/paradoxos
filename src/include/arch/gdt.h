#ifndef _ARCH_GDT_H
#define _ARCH_GDT_H

#include <stdint.h>

struct gdt_descriptor {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

void gdt_init(void);

#endif
