#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct GDT_Descriptor {
    uint16_t limit_15_0;
    uint16_t base_15_0;
    uint8_t  base_23_16;
    uint8_t  type;
    uint8_t  limit_19_16_and_flags;
    uint8_t  base_31_24;
} __attribute__((packed));

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));

struct GDT {
    struct GDT_Descriptor null;
    struct GDT_Descriptor kernel_code;
    struct GDT_Descriptor kernel_data;
    struct GDT_Descriptor user_null;
    struct GDT_Descriptor user_data;
    struct GDT_Descriptor user_code;
    struct GDT_Descriptor tss_low;
    struct GDT_Descriptor tss_high;
} __attribute__((packed));

struct GDT_Pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init(void);

#endif
