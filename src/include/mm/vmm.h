#ifndef _MM_VMM_H
#define _MM_VMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

// Page Table Flags
#define PTE_PRESENT   (1ull << 0)
#define PTE_WRITABLE  (1ull << 1)
#define PTE_USER      (1ull << 2)
#define PTE_NX        (1ull << 63)

// Extended flags
#define PTE_COW       (1ull << 9)   // Copy-on-write flag
#define PTE_SWAPPED   (1ull << 10)  // Page is swapped out

void vmm_init(void);
void vmm_map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags);
void vmm_map_cow_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr);
void vmm_unmap_page(uint64_t *pml4, uint64_t vaddr);
uint64_t *vmm_new_pml4(void);
void vmm_switch_pml4(uint64_t *pml4);
void handle_page_fault(uint64_t vaddr, uint64_t error_code);

// Utility functions
uint64_t get_hhdm_offset(void);
void *phys_to_virt(uint64_t paddr);
uint64_t virt_to_phys(void *vaddr);

#endif
