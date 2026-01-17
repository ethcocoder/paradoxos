#include <kernel.h>
#include <drivers/serial.h>
#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <syscall.h>
#include <gui/window.h>
#include <fs/vfs.h>
#include <app/framework.h>
#include <lib/stdio.h>

// Forward declaration if scheduler header missing
void scheduler_init(void);

void kernel_startup(void) {
    // 0. Arch Init
    gdt_init();
    idt_init();
    serial_init();
    
    printk("[ParadoxOS] CPU Context Initialized (GDT/IDT/Serial).\n");

    // 1. Memory Init
    pmm_init();
    vmm_init();
    
    printk("[ParadoxOS] Memory Initialized (PMM/VMM).\n");

    // 2. Driver Init
    fb_init();
    keyboard_init();
    
    printk("[ParadoxOS] Drivers Initialized (Fb/Kbd).\n");
    
    // 3. Scheduler
    scheduler_init();
    
    // 4. System Calls
    syscall_init();
    
    // 5. File System
    vfs_init();
    
    // 6. GUI System
    wm_init();
    desktop_init();
    
    // 7. Application Framework
    app_framework_init();
    
    printk("[ParadoxOS] Kernel Startup Complete.\n");
    
    // Enable interrupts after all initialization is complete
    __asm__ volatile ("sti");
    printk("[ParadoxOS] Interrupts enabled.\n");
}

void kernel_panic(const char *msg) {
    // Disable interrupts
    __asm__ volatile ("cli");
    
    // Red screen?
    fb_clear(0xFF0000);
    
    printk("KERNEL PANIC: %s\n", msg);
    
    // Halt
    while(1) __asm__ volatile ("hlt");
}
