#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include "../libk/alloc/bitmap.h"
#include "../../boot/limine.h"

#define PAGE_SIZE 4096

typedef struct {
    uint64_t total_memory;
    uint64_t used_pages;
    uint64_t max_pages;
    struct limine_memmap_response *memmap;
} pmm_info_t;

void pmm_init(struct limine_memmap_response *memmap);
void *pmm_alloc(size_t page_count);
void pmm_free(void *ptr, size_t page_count);

#endif
