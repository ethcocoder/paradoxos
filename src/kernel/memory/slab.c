#include "memory/slab.h"
#include "memory/pmm.h"
#include "libk/string/string.h"

static slab_t slabs[SLAB_COUNT];
static void *internal_mem_ptr = NULL;
static size_t internal_mem_len = 0;

static int32_t find_free_object(int32_t slab_index) {
    int32_t objects_per_slab = MAX_SLAB_SIZE / slabs[slab_index].size;
    for (int32_t i = 0; i < objects_per_slab; i++) {
        if (slabs[slab_index].objects[i] != NULL) return i;
    }
    return -1;
}

static int32_t find_allocated_object(int32_t slab_index, void *ptr) {
    int32_t objects_per_slab = MAX_SLAB_SIZE / slabs[slab_index].size;
    for (int32_t i = 0; i < objects_per_slab; i++) {
        // We need to find which object index this pointer corresponds to.
        // Actually, slab_free should just check if it's in range.
        (void)ptr;
        if (slabs[slab_index].objects[i] == NULL) return i;
    }
    return -1;
}

void slab_init(void) {
    for (int32_t i = 0; i < SLAB_COUNT; i++) {
        slabs[i].size = (uint64_t)1 << (i + 1);
        slabs[i].is_full = false;

        int32_t objects_per_slab = MAX_SLAB_SIZE / slabs[i].size;

        for (int j = 0; j < objects_per_slab; j++) {
            if (!internal_mem_ptr || internal_mem_ptr >= (void *)internal_mem_len) {
                internal_mem_ptr = pmm_alloc(1);
                internal_mem_len = (uintptr_t)internal_mem_ptr + PAGE_SIZE;
            }

            slabs[i].objects[j] = internal_mem_ptr;
            internal_mem_ptr = (void *)((uintptr_t)internal_mem_ptr + slabs[i].size);
        }

        slabs[i].address_range.start = slabs[i].objects[0];
        slabs[i].address_range.end = (void *)((uintptr_t)slabs[i].objects[0] + MAX_SLAB_SIZE - slabs[i].size);
    }
}

void *slab_alloc(size_t size) {
    // Find the smallest power of 2 that fits size
    size_t alloc_size = 2;
    int32_t slab_idx = 0;
    while (alloc_size < size && slab_idx < SLAB_COUNT - 1) {
        alloc_size <<= 1;
        slab_idx++;
    }

    if (size > MAX_SLAB_SIZE) return pmm_alloc((size + PAGE_SIZE - 1) / PAGE_SIZE);

    if (slabs[slab_idx].is_full) return NULL;

    int32_t free_idx = find_free_object(slab_idx);
    if (free_idx == -1) {
        slabs[slab_idx].is_full = true;
        return NULL;
    }

    void *res = slabs[slab_idx].objects[free_idx];
    slabs[slab_idx].objects[free_idx] = NULL;
    return res;
}

void slab_free(void *ptr) {
    if (!ptr) return;

    for (int32_t i = 0; i < SLAB_COUNT; i++) {
        if (ptr >= slabs[i].address_range.start && ptr <= slabs[i].address_range.end) {
            // Check if it's aligned to this slab's size
            uintptr_t offset = (uintptr_t)ptr - (uintptr_t)slabs[i].address_range.start;
            if (offset % slabs[i].size == 0) {
                // Return to pool
                int32_t obj_idx = offset / slabs[i].size;
                slabs[i].objects[obj_idx] = ptr;
                slabs[i].is_full = false;
                return;
            }
        }
    }
    
    // If not in slab, it might be a large page allocation
    // (PMM free logic would go here if we tracked large allocations)
}
