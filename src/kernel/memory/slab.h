#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SLAB_COUNT          11    // pow(2, 1) through pow(2, 11)
#define MAX_SLAB_SIZE       2048  // pow(2, 11)
#define MAX_OBJECTS_PER_SLAB 1024 // 2048 / 2

typedef struct {
    void *start;
    void *end;
} addr_range_t;

typedef struct {
    addr_range_t    address_range;
    size_t          size;
    bool            is_full;
    void            *objects[MAX_OBJECTS_PER_SLAB];
} slab_t;

void slab_init(void);
void *slab_alloc(size_t size);
void slab_free(void *ptr);

// Alias to standard names
#define kmalloc(sz) slab_alloc(sz)
#define kfree(ptr) slab_free(ptr)

#endif
