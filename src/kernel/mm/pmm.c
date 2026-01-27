#include <mm/pmm.h>
#include <mm/vmm.h>
#include <limine.h>
#include <lib/string.h>
#include <paradox.h>
#include <drivers/serial.h>

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static uint8_t *bitmap = NULL;
static size_t bitmap_size = 0;
static size_t total_pages = 0;
static size_t free_memory = 0;

static void bitmap_set(size_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static void bitmap_unset(size_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static int bitmap_test(size_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}

void pmm_init(void) {
    if (memmap_request.response == NULL) return;

    struct limine_memmap_entry **entries = memmap_request.response->entries;
    size_t count = memmap_request.response->entry_count;

    // 1. Calculate max memory address to size the bitmap
    uint64_t highest_addr = 0;
    for (size_t i = 0; i < count; i++) {
        uint64_t top = entries[i]->base + entries[i]->length;
        if (top > highest_addr) highest_addr = top;
    }

    total_pages = highest_addr / PAGE_SIZE;
    bitmap_size = ALIGN_UP(total_pages / 8, PAGE_SIZE);

    // 2. Find a place to store the bitmap
    // We look for a Usable Memory region large enough to hold the bitmap
    for (size_t i = 0; i < count; i++) {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE && entries[i]->length >= bitmap_size) {
             // Use HHDM for bitmap access
             bitmap = (uint8_t *)phys_to_virt(entries[i]->base);
             
             // Initialize bitmap to all used (1) initially, then free available regions
             memset(bitmap, 0xFF, bitmap_size);
             
             // Mark the bitmap region itself as used
             // (This will be refined in step 3/4)
             break;
        }
    }
    
    // 3. Mark usable regions as free (0) in bitmap
     for (size_t i = 0; i < count; i++) {
         if (entries[i]->type == LIMINE_MEMMAP_USABLE) {
             uint64_t start_frame = entries[i]->base / PAGE_SIZE;
             uint64_t end_frame = (entries[i]->base + entries[i]->length) / PAGE_SIZE;
             
             for (uint64_t j = start_frame; j < end_frame; j++) {
                 bitmap_unset(j);
             }
         }
     }
     
     // 4. Mark bitmap itself as used
     // TODO to be perfect
     
     serial_puts("[PMM] Physical Memory Manager Initialized.\n");
}

// Simple alloc for now (First Fit)
void *pmm_alloc(size_t count) {
    size_t contiguous = 0;
    size_t start_frame = 0;
    
    for (size_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            if (contiguous == 0) start_frame = i;
            contiguous++;
            if (contiguous == count) {
                // Found run
                for (size_t j = start_frame; j < start_frame + count; j++) {
                    bitmap_set(j);
                }
                return (void *)(uint64_t)(start_frame * PAGE_SIZE);
            }
        } else {
            contiguous = 0;
        }
    }
    return NULL; // OOM
}

void pmm_free(void *paddr, size_t count) {
    uint64_t frame = (uint64_t)paddr / PAGE_SIZE;
    for (size_t i = frame; i < frame + count; i++) {
        bitmap_unset(i);
    }
}
