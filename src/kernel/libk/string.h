#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

void *k_memset(void *s, int c, size_t n);
void *k_memcpy(void *dest, const void *src, size_t n);
size_t k_strlen(const char *s);

#endif
