#include "libk/alloc/bitmap.h"

void bitmap_set_bit(bitmap_t *bitmap, int bit) {
    bitmap->map[bit / 8] |= (1 << (bit % 8));
}

void bitmap_unset_bit(bitmap_t *bitmap, int bit) {
    bitmap->map[bit / 8] &= ~(1 << (bit % 8));
}

uint8_t bitmap_check_bit(bitmap_t *bitmap, int bit) {
    return bitmap->map[bit / 8] & (1 << (bit % 8));
}
