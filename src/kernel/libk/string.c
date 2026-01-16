#include "string.h"

void *k_memset(void *s, int c, size_t n) {
    uint8_t *p = s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

void *k_memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = dest;
    const uint8_t *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

size_t k_strlen(const char *s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}
