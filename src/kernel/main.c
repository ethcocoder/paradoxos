#include <paradox.h>
#include <kernel.h>
#include <lib/stdio.h>
#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <gui/window.h>
#include <app/framework.h>
#include <sched/scheduler.h>
#include <limine.h>

// Helper to catch compiler inserted calls (if any)
void *__dso_handle;

// Define Limine Base Revision
LIMINE_BASE_REVISION(1)

// Forward declarations
void desktop_task(void);
void task_entry_b(void);
void display_system_status(void);

// Desktop task
void desktop_task(void) {
    static int mouse_x_prev = -1, mouse_y_prev = -1;
    static bool left_prev = false;
    
    while (1) {
        // Handle input
        int mouse_x, mouse_y;
        bool left, right, middle;
        get_mouse_state(&mouse_x, &mouse_y, &left, &right, &middle);
        
        // Handle mouse clicks
        if (left && !left_prev) {
            desktop_handle_mouse(mouse_x, mouse_y, true, false, false);
        }
        
        // Handle keyboard input
        char key = keyboard_get_key();
        if (key) {
            desktop_handle_keyboard(key);
        }
        
        // Render desktop if mouse moved or input occurred
        if (mouse_x != mouse_x_prev || mouse_y != mouse_y_prev || key || left != left_prev) {
            desktop_render();
            mouse_x_prev = mouse_x;
            mouse_y_prev = mouse_y;
            left_prev = left;
        }
        
        // Run applications
        app_run_all();
        
        schedule();
        
        // Small delay to prevent excessive CPU usage
        for (volatile int i = 0; i < 10000; i++);
    }
}

// Test Task
void task_entry_b(void) {
    int counter = 0;
    while (1) {
        if (counter % 1000 == 0) {
            printk("Background task B running... (%d)\n", counter / 1000);
        }
        counter++;
        schedule();
        for (volatile int i=0; i<100000; i++); // slowdown
    }
}

void _start(void) {
    // Initialize System
    kernel_startup();

    // Clear screen and show boot message
    fb_clear(0x1a1a1a);
    fb_draw_string(50, 50, "ParadoxOS - Booting...", COLOR_WHITE, 0x1a1a1a);
    fb_swap_buffers();
    
    printk("[ParadoxOS] Starting built-in applications...\n");
    
    // Applications are automatically started by the framework
    // No need to create demo windows manually anymore
    
    // Display comprehensive system status
    display_system_status();
    
    // Create desktop task
    task_create(desktop_task);
    
    // Create background task
    task_create(task_entry_b);
    
    printk("[ParadoxOS] Entering Main Loop - Desktop Environment Active\n");

    // Main kernel task - system monitoring
    int tick = 0;
    while (1) {
        tick++;
        
        // System monitoring every 10 seconds
        if (tick % 10000 == 0) {
            printk("[System] Tick %d - System running normally\n", tick / 1000);
        }
        
        schedule(); 
        for (volatile int i=0; i<100000; i++);
    }
}
