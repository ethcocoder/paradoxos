#include <mm/vmm.h>
#include <mm/pmm.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <limine.h>
#include <stdbool.h>

// Helper to get index from virtual address
#define PML4_INDEX(va) (((va) >> 39) & 0x1FF)
#define PDP_INDEX(va)  (((va) >> 30) & 0x1FF)
#define PD_INDEX(va)   (((va) >> 21) & 0x1FF)
#define PT_INDEX(va)   (((va) >> 12) & 0x1FF)

// Additional page flags
#define PTE_COW       (1ull << 9)   // Copy-on-write flag (available bit)
#define PTE_SWAPPED   (1ull << 10)  // Page is swapped out (available bit)

static uint64_t *kernel_pml4 = NULL;
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

// Simple swap file simulation (in-memory for now)
#define SWAP_PAGES 1024
static uint8_t swap_storage[SWAP_PAGES * PAGE_SIZE];
static bool swap_used[SWAP_PAGES];
static size_t swap_next = 0;

// Memory pressure tracking
static size_t total_mapped_pages = 0;
static size_t max_mapped_pages = 0x10000; // 256MB limit for demo

uint64_t get_hhdm_offset(void) {
    if (hhdm_request.response) {
        return hhdm_request.response->offset;
    }
    return 0xffff800000000000; // Default HHDM offset
}

void *phys_to_virt(uint64_t paddr) {
    return (void *)(paddr + get_hhdm_offset());
}

uint64_t virt_to_phys(void *vaddr) {
    return (uint64_t)vaddr - get_hhdm_offset();
}

int swap_out_page(uint64_t vaddr) {
    // Find free swap slot
    for (size_t i = 0; i < SWAP_PAGES; i++) {
        size_t slot = (swap_next + i) % SWAP_PAGES;
        if (!swap_used[slot]) {
            // Get physical address of page
            uint64_t *pml4 = kernel_pml4;
            uint64_t pml4_idx = PML4_INDEX(vaddr);
            uint64_t pdp_idx = PDP_INDEX(vaddr);
            uint64_t pd_idx = PD_INDEX(vaddr);
            uint64_t pt_idx = PT_INDEX(vaddr);
            
            if (!(pml4[pml4_idx] & PTE_PRESENT)) return -1;
            uint64_t *pdp = phys_to_virt(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000);
            
            if (!(pdp[pdp_idx] & PTE_PRESENT)) return -1;
            uint64_t *pd = phys_to_virt(pdp[pdp_idx] & 0xFFFFFFFFFFFFF000);
            
            if (!(pd[pd_idx] & PTE_PRESENT)) return -1;
            uint64_t *pt = phys_to_virt(pd[pd_idx] & 0xFFFFFFFFFFFFF000);
            
            if (!(pt[pt_idx] & PTE_PRESENT)) return -1;
            
            uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000;
            void *page_data = phys_to_virt(paddr);
            
            // Copy page to swap
            memcpy(&swap_storage[slot * PAGE_SIZE], page_data, PAGE_SIZE);
            swap_used[slot] = true;
            swap_next = (slot + 1) % SWAP_PAGES;
            
            // Mark page as swapped and free physical memory
            pt[pt_idx] = (slot << 12) | PTE_SWAPPED;
            pmm_free(phys_to_virt(paddr), 1);
            
            // Invalidate TLB
            __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
            
            total_mapped_pages--;
            return slot;
        }
    }
    return -1; // No swap space available
}

int swap_in_page(uint64_t vaddr, int swap_slot) {
    // Allocate new physical page
    void *new_page = pmm_alloc(1);
    if (!new_page) return -1;
    
    // Copy data from swap
    memcpy(new_page, &swap_storage[swap_slot * PAGE_SIZE], PAGE_SIZE);
    swap_used[swap_slot] = false;
    
    // Update page table
    uint64_t *pml4 = kernel_pml4;
    uint64_t pml4_idx = PML4_INDEX(vaddr);
    uint64_t pdp_idx = PDP_INDEX(vaddr);
    uint64_t pd_idx = PD_INDEX(vaddr);
    uint64_t pt_idx = PT_INDEX(vaddr);
    
    uint64_t *pdp = phys_to_virt(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000);
    uint64_t *pd = phys_to_virt(pdp[pdp_idx] & 0xFFFFFFFFFFFFF000);
    uint64_t *pt = phys_to_virt(pd[pd_idx] & 0xFFFFFFFFFFFFF000);
    
    pt[pt_idx] = virt_to_phys(new_page) | PTE_PRESENT | PTE_WRITABLE;
    
    // Invalidate TLB
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
    
    total_mapped_pages++;
    return 0;
}

void handle_page_fault(uint64_t vaddr, uint64_t error_code) {
    uint64_t *pml4 = kernel_pml4;
    uint64_t pml4_idx = PML4_INDEX(vaddr);
    uint64_t pdp_idx = PDP_INDEX(vaddr);
    uint64_t pd_idx = PD_INDEX(vaddr);
    uint64_t pt_idx = PT_INDEX(vaddr);
    
    // Check if page tables exist
    if (!(pml4[pml4_idx] & PTE_PRESENT)) {
        printk("Page fault: PML4 entry not present for 0x%lx\n", vaddr);
        return;
    }
    
    uint64_t *pdp = phys_to_virt(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000);
    if (!(pdp[pdp_idx] & PTE_PRESENT)) {
        printk("Page fault: PDP entry not present for 0x%lx\n", vaddr);
        return;
    }
    
    uint64_t *pd = phys_to_virt(pdp[pdp_idx] & 0xFFFFFFFFFFFFF000);
    if (!(pd[pd_idx] & PTE_PRESENT)) {
        printk("Page fault: PD entry not present for 0x%lx\n", vaddr);
        return;
    }
    
    uint64_t *pt = phys_to_virt(pd[pd_idx] & 0xFFFFFFFFFFFFF000);
    uint64_t pte = pt[pt_idx];
    
    // Handle swapped page
    if (pte & PTE_SWAPPED) {
        int swap_slot = (pte >> 12) & 0xFFFFF;
        printk("Page fault: Swapping in page from slot %d\n", swap_slot);
        if (swap_in_page(vaddr, swap_slot) == 0) {
            return; // Successfully swapped in
        }
    }
    
    // Handle copy-on-write
    if ((pte & PTE_COW) && (error_code & 0x2)) { // Write to COW page
        uint64_t old_paddr = pte & 0xFFFFFFFFFFFFF000;
        void *new_page = pmm_alloc(1);
        if (new_page) {
            // Copy old page content
            memcpy(new_page, phys_to_virt(old_paddr), PAGE_SIZE);
            
            // Update page table entry
            pt[pt_idx] = virt_to_phys(new_page) | PTE_PRESENT | PTE_WRITABLE;
            
            // Invalidate TLB
            __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
            
            printk("Page fault: COW handled for 0x%lx\n", vaddr);
            return;
        }
    }
    
    // Handle demand paging - allocate new page
    if (!(pte & PTE_PRESENT)) {
        // Check memory pressure
        if (total_mapped_pages >= max_mapped_pages) {
            // Try to swap out a page
            // For simplicity, swap out a random page
            uint64_t victim_addr = 0x400000 + (total_mapped_pages % 100) * PAGE_SIZE;
            if (swap_out_page(victim_addr) < 0) {
                printk("Page fault: Out of memory and swap space\n");
                return;
            }
        }
        
        void *new_page = pmm_alloc(1);
        if (new_page) {
            memset(new_page, 0, PAGE_SIZE);
            pt[pt_idx] = virt_to_phys(new_page) | PTE_PRESENT | PTE_WRITABLE;
            
            // Invalidate TLB
            __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
            
            total_mapped_pages++;
            printk("Page fault: Allocated new page for 0x%lx\n", vaddr);
            return;
        }
    }
    
    printk("Page fault: Unable to handle fault at 0x%lx (error: 0x%lx)\n", vaddr, error_code);
}

void vmm_map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t pml4_idx = PML4_INDEX(vaddr);
    uint64_t pdp_idx  = PDP_INDEX(vaddr);
    uint64_t pd_idx   = PD_INDEX(vaddr);
    uint64_t pt_idx   = PT_INDEX(vaddr);

    // 1. Check PML4 Entry
    if (!(pml4[pml4_idx] & PTE_PRESENT)) {
        uint64_t *new_pdp = (uint64_t *)pmm_alloc(1);
        if (!new_pdp) return;
        memset(new_pdp, 0, PAGE_SIZE);
        pml4[pml4_idx] = virt_to_phys(new_pdp) | PTE_PRESENT | PTE_WRITABLE;
    }
    uint64_t *pdp = phys_to_virt(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000);

    // 2. Check PDP Entry
    if (!(pdp[pdp_idx] & PTE_PRESENT)) {
        uint64_t *new_pd = (uint64_t *)pmm_alloc(1);
        if (!new_pd) return;
        memset(new_pd, 0, PAGE_SIZE);
        pdp[pdp_idx] = virt_to_phys(new_pd) | PTE_PRESENT | PTE_WRITABLE;
    }
    uint64_t *pd = phys_to_virt(pdp[pdp_idx] & 0xFFFFFFFFFFFFF000);

    // 3. Check PD Entry
    if (!(pd[pd_idx] & PTE_PRESENT)) {
        uint64_t *new_pt = (uint64_t *)pmm_alloc(1);
        if (!new_pt) return;
        memset(new_pt, 0, PAGE_SIZE);
        pd[pd_idx] = virt_to_phys(new_pt) | PTE_PRESENT | PTE_WRITABLE;
    }
    uint64_t *pt = phys_to_virt(pd[pd_idx] & 0xFFFFFFFFFFFFF000);

    // 4. Set Page Table Entry
    pt[pt_idx] = (paddr & 0xFFFFFFFFFFFFF000) | (flags & 0xFFF) | PTE_PRESENT;
    
    // Invalidate TLB
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
    
    total_mapped_pages++;
}

void vmm_map_cow_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr) {
    vmm_map_page(pml4, vaddr, paddr, PTE_COW); // Read-only, copy-on-write
}

uint64_t *vmm_new_pml4(void) {
    uint64_t *pml4 = (uint64_t *)pmm_alloc(1);
    if (!pml4) return NULL;
    memset(pml4, 0, PAGE_SIZE);
    
    // Copy kernel mappings from current PML4 if available
    if (kernel_pml4) {
        // Copy higher-half kernel mappings (entries 256-511)
        for (int i = 256; i < 512; i++) {
            pml4[i] = kernel_pml4[i];
        }
    }
    
    return pml4;
}

void vmm_switch_pml4(uint64_t *pml4) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(virt_to_phys(pml4)) : "memory");
}

void vmm_unmap_page(uint64_t *pml4, uint64_t vaddr) {
    uint64_t pml4_idx = PML4_INDEX(vaddr);
    uint64_t pdp_idx = PDP_INDEX(vaddr);
    uint64_t pd_idx = PD_INDEX(vaddr);
    uint64_t pt_idx = PT_INDEX(vaddr);
    
    if (!(pml4[pml4_idx] & PTE_PRESENT)) return;
    uint64_t *pdp = phys_to_virt(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000);
    
    if (!(pdp[pdp_idx] & PTE_PRESENT)) return;
    uint64_t *pd = phys_to_virt(pdp[pdp_idx] & 0xFFFFFFFFFFFFF000);
    
    if (!(pd[pd_idx] & PTE_PRESENT)) return;
    uint64_t *pt = phys_to_virt(pd[pd_idx] & 0xFFFFFFFFFFFFF000);
    
    uint64_t pte = pt[pt_idx];
    if (pte & PTE_PRESENT) {
        uint64_t paddr = pte & 0xFFFFFFFFFFFFF000;
        pmm_free(phys_to_virt(paddr), 1);
        total_mapped_pages--;
    }
    
    pt[pt_idx] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

void vmm_init(void) {
    // Get current page table from CR3
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    kernel_pml4 = phys_to_virt(cr3);
    
    // Initialize swap system
    memset(swap_used, 0, sizeof(swap_used));
    
    printk("[VMM] Virtual Memory Manager initialized\n");
    printk("[VMM] HHDM offset: 0x%lx\n", get_hhdm_offset());
    printk("[VMM] Kernel PML4: 0x%p\n", kernel_pml4);
    printk("[VMM] Demand paging and COW support enabled\n");
}
