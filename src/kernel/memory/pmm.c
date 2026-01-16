#include "memory/pmm.h"
#include "libk/string/string.h"

static pmm_info_t pmm_info;
static bitmap_t bitmap;
static uint64_t hhdm_offset = 0;
static uint64_t highest_address = 0;


extern volatile struct limine_hhdm_request hhdm_request;

void pmm_init(struct limine_memmap_response *memmap_response) {
    if (hhdm_request.response) {
        hhdm_offset = hhdm_request.response->offset;
    }

    pmm_info.memmap = memmap_response;
    
    // Find highest address to determine bitmap size
    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE || 
            entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
            entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            
            uint64_t top = entry->base + entry->length;
            if (top > highest_address) highest_address = top;
        }
    }

    pmm_info.total_memory = highest_address;
    pmm_info.max_pages = highest_address / PAGE_SIZE;
    pmm_info.used_pages = pmm_info.max_pages; // Start all as used

    // Calculate bitmap size (1 bit per page)
    size_t bitmap_size = (pmm_info.max_pages / 8) + 1;
    // Round up to page size
    bitmap_size = (bitmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // Find a block for the bitmap
    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            bitmap.map = (uint8_t *)(entry->base + hhdm_offset);
            bitmap.size = bitmap_size;
            
            // Mark the bitmap memory as used by effectively removing it from usable memory later
            entry->base += bitmap_size;
            entry->length -= bitmap_size;
            break;
        }
    }

    // Initialize bitmap: everything is used (1)
    k_memset(bitmap.map, 0xFF, bitmap.size);

    // Free usable regions in bitmap (set to 0)
    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
                uint64_t page = (entry->base + j) / PAGE_SIZE;
                if (page < pmm_info.max_pages) {
                    bitmap_unset_bit(&bitmap, page);
                    pmm_info.used_pages--;
                }
            }
        }
    }

    // Reserve page 0 (null) to be safe
    bitmap_set_bit(&bitmap, 0);
}

void *pmm_alloc(size_t page_count) {
    if (page_count == 0) return NULL;

    size_t consecutive = 0;
    size_t start_page = 0;

    for (uint64_t i = 0; i < pmm_info.max_pages; i++) {
        if (!bitmap_check_bit(&bitmap, i)) {
            if (consecutive == 0) start_page = i;
            consecutive++;
            if (consecutive == page_count) {
                // Found it
                for (size_t j = 0; j < page_count; j++) {
                    bitmap_set_bit(&bitmap, start_page + j);
                }
                pmm_info.used_pages += page_count;
                return (void *)(start_page * PAGE_SIZE + hhdm_offset);
            }
        } else {
            consecutive = 0;
        }
    }

    return NULL; // Out of memory
}

void pmm_free(void *ptr, size_t page_count) {
    if (!ptr) return;
    uint64_t addr = (uintptr_t)ptr - hhdm_offset;
    uint64_t start_page = addr / PAGE_SIZE;

    for (size_t i = 0; i < page_count; i++) {
        if (bitmap_check_bit(&bitmap, start_page + i)) {
            bitmap_unset_bit(&bitmap, start_page + i);
            pmm_info.used_pages--;
        }
    }
}
