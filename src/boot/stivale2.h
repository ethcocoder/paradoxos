#ifndef __STIVALE__STIVALE2_H__
#define __STIVALE__STIVALE2_H__

/*
 * stivale2.h - stivale2 protocol header
 * Imported from KnutOS for ParadoxOS Assimilation
 */

#include <stdint.h>

struct stivale2_tag {
    uint64_t identifier;
    uint64_t next;
} __attribute__((__packed__));

struct stivale2_header {
    uint64_t entry_point;
    uint64_t stack;
    uint64_t flags;
    uint64_t tags;
} __attribute__((__packed__));

#define STIVALE2_HEADER_TAG_FRAMEBUFFER_ID 0x3ecc1bc43d0f7971

struct stivale2_header_tag_framebuffer {
    struct stivale2_tag tag;
    uint16_t framebuffer_width;
    uint16_t framebuffer_height;
    uint16_t framebuffer_bpp;
} __attribute__((__packed__));

struct stivale2_struct {
    char bootloader_brand[64];
    char bootloader_version[64];
    uint64_t tags;
} __attribute__((__packed__));

#define STIVALE2_STRUCT_TAG_MEMMAP_ID 0x2187f79e8612de07
#define STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID 0x506461d2950408fa

struct stivale2_mmap_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
} __attribute__((__packed__));

struct stivale2_struct_tag_memmap {
    struct stivale2_tag tag;
    uint64_t entries;
    struct stivale2_mmap_entry memmap[];
} __attribute__((__packed__));

struct stivale2_struct_tag_framebuffer {
    struct stivale2_tag tag;
    uint64_t framebuffer_addr;
    uint16_t framebuffer_width;
    uint16_t framebuffer_height;
    uint16_t framebuffer_pitch;
    uint16_t framebuffer_bpp;
    uint8_t  memory_model;
    uint8_t  red_mask_size;
    uint8_t  red_mask_shift;
    uint8_t  green_mask_size;
    uint8_t  green_mask_shift;
    uint8_t  blue_mask_size;
    uint8_t  blue_mask_shift;
} __attribute__((__packed__));

#endif
