#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <boot/stivale2.h>
#include <boot/stivale2_boot.h>
#include <memory/mem.h>
#include <memory/pmm.h>
#include <memory/slab.h>
#include <libk/debug/debug.h>
#include <libk/log/log.h>
#include <libk/math/math.h>
#include <libk/stdio/stdio.h>

static slab_t slabs[SLAB_COUNT];

static int32_t find_free_object(int32_t slab_index);
static int32_t find_allocated_object(int32_t slab_index);

void *internal_mem_ptr = NULL;
size_t internal_mem_len = 0;
uint32_t page_alloc_count = 0;

void slab_init(void)
{
    for (int32_t i = 0; i < SLAB_COUNT; i++)
    {
        slabs[i].size = pow(2, i + 1);
        slabs[i].is_full = false;

        int32_t objects_per_slab = MAX_SLAB_SIZE / pow(2, i + 1);

        for (int j = 0; j < objects_per_slab; j++)
        {
            if (!internal_mem_ptr || internal_mem_ptr == (void *)internal_mem_len)
            {
                internal_mem_ptr = pmm_alloc(1);
                internal_mem_len = (size_t)internal_mem_ptr + PAGE_SIZE;

                page_alloc_count++;
            }

            slabs[i].objects[j] = internal_mem_ptr;
            internal_mem_ptr += slabs[i].size;
        }

        slabs[i].address_range.start = slabs[i].objects[0];
        slabs[i].address_range.end = (void*)((uintptr_t)slabs[i].objects[0] + 1024 - slabs[i].size);
    }
}

void *slab_alloc(size_t size)
{
    void *return_value = NULL;

    for (int32_t i = 0; i < SLAB_COUNT; i++)
    {
        if (slabs[i].size != size)
            continue;

        if (slabs[i].is_full)
            return return_value;

        int32_t free_object_index = find_free_object(i);

        if (free_object_index == -1)
            slabs[i].is_full = true;
        else
        {
            return_value = slabs[i].objects[free_object_index];
            slabs[i].objects[free_object_index] = NULL;
        }

        return return_value;
    }

    return return_value;
}

void slab_free(void *ptr)
{
    if (!ptr)
        return;

    void *ptr_without_base;

    for (int32_t i = 0; i < SLAB_COUNT; i++)
    {
        ptr_without_base = (void*)((uintptr_t)ptr - (uintptr_t)slabs[i].address_range.start + slabs[i].size);

        if (ptr >= slabs[i].address_range.start && ptr <= slabs[i].address_range.end)
        {
            if ((uintptr_t)ptr_without_base % slabs[i].size == 0)
            {
                int32_t allocated_object_index = find_allocated_object(i);

                if (allocated_object_index == -1)
                    return;

                slabs[i].objects[allocated_object_index] = ptr;
            }
        }
    }
}

static int32_t find_free_object(int32_t slab_index)
{
    int32_t objects_per_slab = MAX_SLAB_SIZE / pow(2, slab_index + 1);

    for (int32_t i = 0; i < objects_per_slab; i++)
        if (slabs[slab_index].objects[i] != NULL)
            return i;

    return -1;
}

static int32_t find_allocated_object(int32_t slab_index)
{
    int32_t objects_per_slab = MAX_SLAB_SIZE / pow(2, slab_index + 1);

    for (int32_t i = 0; i < objects_per_slab; i++)
        if (slabs[slab_index].objects[i] == NULL)
            return i;

    return -1;
}
