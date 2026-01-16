#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t *map;
    size_t size;
} bitmap_t;

void bitmap_set_bit(bitmap_t *bitmap, int bit);
void bitmap_unset_bit(bitmap_t *bitmap, int bit);
uint8_t bitmap_check_bit(bitmap_t *bitmap, int bit);

#endif
