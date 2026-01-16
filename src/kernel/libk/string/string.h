#ifndef STRING_K_H
#define STRING_K_H

#include <stddef.h>
#include <stdint.h>

void *k_memset(void *s, int c, size_t n);
void *k_memcpy(void *dest, const void *src, size_t n);

#endif
