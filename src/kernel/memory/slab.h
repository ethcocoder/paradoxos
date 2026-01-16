#include <stdbool.h>
#include <libk/definitions.h>

#ifndef SLAB_H
#define SLAB_H

#define SLAB_COUNT		11
#define MAX_SLAB_SIZE		2048
#define MAX_OBJECTS_PER_SLAB	1024

typedef struct
{
    addr_range_t    address_range;

    size_t  size;
    bool    is_full;
    void    *objects[MAX_OBJECTS_PER_SLAB];
} slab_t;

void slab_init(void);
void *slab_alloc(size_t size);
void slab_free(void *ptr);

#endif
