
#include <memory/mem.h>

#ifndef VMM_H
#define VMM_H

#define GB 0x40000000UL

// page table entry flags
#define PTE_PRESENT	    1
#define PTE_READ_WRITE	    2
#define PTE_USER_SUPERVISOR 4
#define PTE_WRITE_THROUGH   8
#define PTE_CHACHE_DISABLED 16
#define PTE_ACCESSED	    32
#define PTE_DIRTY	    64
#define PTE_PAT		    128
#define PTE_GLOBAL	    256

typedef uint64_t *PAGE_DIR;

bool is_la57_enabled(void);
void vmm_init(void);
PAGE_DIR vmm_create_page_directory(void);
void vmm_map_page(PAGE_DIR vmm, uintptr_t physical_address, uintptr_t virtual_address, int flags);
void vmm_unmap_page(PAGE_DIR current_page_directory, uintptr_t virtual_address);
void vmm_flush_tlb(void *address);
void vmm_activate_page_directory(PAGE_DIR vmm);

#endif
