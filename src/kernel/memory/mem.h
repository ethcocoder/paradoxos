#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdbool.h>

#define HIGHER_HALF_DATA  0xFFFF800000000000UL
#define HIGHER_HALF_CODE  0xFFFFFFFF80000000UL
#define PAGE_SIZE         4096

static inline uintptr_t phys_to_virt_data(uintptr_t phys) {
    return phys + HIGHER_HALF_DATA;
}

static inline uintptr_t virt_to_phys_data(uintptr_t virt) {
    return virt - HIGHER_HALF_DATA;
}

#endif
