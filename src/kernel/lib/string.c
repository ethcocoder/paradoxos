#include <lib/string.h>

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pmax = (uint8_t *)dest + n;
    const uint8_t *p_src = (const uint8_t *)src;
    uint8_t *p_dest = (uint8_t *)dest;

    while (p_dest != pmax) {
        *p_dest++ = *p_src++;
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *p_dest = (uint8_t *)dest;
    const uint8_t *p_src = (const uint8_t *)src;

    if (p_src < p_dest && p_src + n > p_dest) {
        p_dest += n;
        p_src += n;
        while (n--) {
            *--p_dest = *--p_src;
        }
    } else {
        memcpy(dest, src, n);
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len])
        len++;
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *orig_dest = dest;
    while ((*dest++ = *src++));
    return orig_dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) {
            return (char *)s;
        }
        s++;
    }
    return (c == '\0') ? (char *)s : NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) {
        if (*s == c) {
            last = s;
        }
        s++;
    }
    if (c == '\0') {
        return (char *)s;
    }
    return (char *)last;
}
