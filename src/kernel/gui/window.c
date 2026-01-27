#include <gui/window.h>
#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <mm/pmm.h>

// Window manager state
static window_t windows[MAX_WINDOWS];
static uint32_t next_window_id = 1;
static uint32_t focused_window = 0;
static bool wm_initialized = false;

// Desktop state
static bool desktop_initialized = false;
static uint32_t desktop_color = 0x2D2D30; // Dark gray
static char *wallpaper_text = "ParadoxOS Desktop";

// Taskbar constants
#define TASKBAR_HEIGHT 40
#define TASKBAR_COLOR 0x1E1E1E
#define TASKBAR_TEXT_COLOR 0xFFFFFF
#define START_BUTTON_WIDTH 100

void wm_init(void) {
    memset(windows, 0, sizeof(windows));
    next_window_id = 1;
    focused_window = 0;
    wm_initialized = true;
    
    printk("[WM] Window Manager initialized\n");
}

uint32_t wm_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, 
                         const char *title, uint32_t flags) {
    if (!wm_initialized) return 0;
    
    // Find free window slot
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == 0) {
            window_t *win = &windows[i];
            
            win->id = next_window_id++;
            win->x = x;
            win->y = y;
            win->width = width;
            win->height = height;
            win->z_order = next_window_id; // Higher z_order = on top
            win->state = WINDOW_STATE_NORMAL;
            win->flags = flags | WINDOW_FLAG_VISIBLE;
            win->needs_redraw = true;
            win->owner_pid = 1; // TODO: Get actual PID
            
            if (title) {
                strncpy(win->title, title, WINDOW_TITLE_MAX - 1);
                win->title[WINDOW_TITLE_MAX - 1] = '\0';
            } else {
                strcpy(win->title, "Untitled Window");
            }
            
            // Allocate window buffer
            size_t buffer_size = width * height * sizeof(uint32_t);
            size_t pages_needed = (buffer_size + PAGE_SIZE - 1) / PAGE_SIZE;
            void *phys_buffer = pmm_alloc(pages_needed);
            if (phys_buffer) {
                win->buffer = (uint32_t *)phys_to_virt((uint64_t)phys_buffer);
            } else {
                win->buffer = NULL;
            }
            
            if (win->buffer) {
                // Clear window buffer
                for (uint32_t i = 0; i < width * height; i++) {
                    win->buffer[i] = 0xF0F0F0; // Light gray background
                }
            }
            
            // Focus the new window
            wm_focus_window(win->id);
            
            printk("[WM] Created window %u: '%s' at (%u,%u) %ux%u\n", 
                   win->id, win->title, x, y, width, height);
            
            return win->id;
        }
    }
    
    printk("[WM] Error: No free window slots\n");
    return 0;
}

void wm_destroy_window(uint32_t window_id) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            window_t *win = &windows[i];
            
            // Free window buffer
            if (win->buffer) {
                size_t buffer_size = win->width * win->height * sizeof(uint32_t);
                size_t pages_needed = (buffer_size + PAGE_SIZE - 1) / PAGE_SIZE;
                pmm_free((void *)virt_to_phys(win->buffer), pages_needed);
            }
            
            printk("[WM] Destroyed window %u\n", window_id);
            
            // Clear window slot
            memset(win, 0, sizeof(window_t));
            
            // Focus another window if this was focused
            if (focused_window == window_id) {
                focused_window = 0;
                // Find another window to focus
                for (int j = 0; j < MAX_WINDOWS; j++) {
                    if (windows[j].id != 0 && (windows[j].flags & WINDOW_FLAG_VISIBLE)) {
                        wm_focus_window(windows[j].id);
                        break;
                    }
                }
            }
            return;
        }
    }
}

void wm_move_window(uint32_t window_id, uint32_t x, uint32_t y) {
    window_t *win = wm_get_window(window_id);
    if (win) {
        win->x = x;
        win->y = y;
        win->needs_redraw = true;
    }
}

void wm_resize_window(uint32_t window_id, uint32_t width, uint32_t height) {
    window_t *win = wm_get_window(window_id);
    if (win && (win->flags & WINDOW_FLAG_RESIZABLE)) {
        // TODO: Reallocate buffer if size changed significantly
        win->width = width;
        win->height = height;
        win->needs_redraw = true;
    }
}

void wm_set_window_state(uint32_t window_id, window_state_t state) {
    window_t *win = wm_get_window(window_id);
    if (win) {
        win->state = state;
        win->needs_redraw = true;
        
        if (state == WINDOW_STATE_MINIMIZED) {
            win->flags &= ~WINDOW_FLAG_VISIBLE;
        } else {
            win->flags |= WINDOW_FLAG_VISIBLE;
        }
    }
}

void wm_focus_window(uint32_t window_id) {
    window_t *win = wm_get_window(window_id);
    if (win) {
        // Remove focus from current window
        if (focused_window != 0) {
            window_t *old_focused = wm_get_window(focused_window);
            if (old_focused) {
                old_focused->flags &= ~WINDOW_FLAG_FOCUSED;
                old_focused->needs_redraw = true;
            }
        }
        
        // Set new focused window
        focused_window = window_id;
        win->flags |= WINDOW_FLAG_FOCUSED;
        win->needs_redraw = true;
        
        // Bring to front (highest z_order)
        uint32_t max_z = 0;
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (windows[i].id != 0 && windows[i].z_order > max_z) {
                max_z = windows[i].z_order;
            }
        }
        win->z_order = max_z + 1;
    }
}

window_t *wm_get_window(uint32_t window_id) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            return &windows[i];
        }
    }
    return NULL;
}

void wm_render_window(window_t *win) {
    if (!win || !(win->flags & WINDOW_FLAG_VISIBLE) || win->state == WINDOW_STATE_MINIMIZED) {
        return;
    }
    
    // Draw window frame
    uint32_t frame_color = (win->flags & WINDOW_FLAG_FOCUSED) ? 0x0078D4 : 0x404040;
    
    // Title bar
    fb_draw_rect(win->x, win->y, win->width, 30, frame_color);
    fb_draw_string(win->x + 8, win->y + 8, win->title, 0xFFFFFF, frame_color);
    
    // Close button
    if (win->flags & WINDOW_FLAG_CLOSABLE) {
        fb_draw_rect(win->x + win->width - 25, win->y + 5, 20, 20, 0xFF4444);
        fb_draw_string(win->x + win->width - 20, win->y + 8, "X", 0xFFFFFF, 0xFF4444);
    }
    
    // Window content area
    if (win->buffer) {
        // Copy window buffer to framebuffer
        for (uint32_t y = 0; y < win->height - 30; y++) {
            for (uint32_t x = 0; x < win->width; x++) {
                if (win->x + x < 1920 && win->y + 30 + y < 1080) {
                    uint32_t pixel = win->buffer[y * win->width + x];
                    fb_put_pixel(win->x + x, win->y + 30 + y, pixel);
                }
            }
        }
    } else {
        // Draw default content
        fb_draw_rect(win->x, win->y + 30, win->width, win->height - 30, 0xF0F0F0);
    }
    
    // Window border
    fb_draw_rect(win->x - 1, win->y - 1, win->width + 2, 1, frame_color); // Top
    fb_draw_rect(win->x - 1, win->y + win->height, win->width + 2, 1, frame_color); // Bottom
    fb_draw_rect(win->x - 1, win->y, 1, win->height, frame_color); // Left
    fb_draw_rect(win->x + win->width, win->y, 1, win->height, frame_color); // Right
    
    win->needs_redraw = false;
}

void wm_render_all(void) {
    if (!wm_initialized) return;
    
    // Sort windows by z_order (back to front)
    window_t *sorted_windows[MAX_WINDOWS];
    int window_count = 0;
    
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id != 0) {
            sorted_windows[window_count++] = &windows[i];
        }
    }
    
    // Simple bubble sort by z_order
    for (int i = 0; i < window_count - 1; i++) {
        for (int j = 0; j < window_count - i - 1; j++) {
            if (sorted_windows[j]->z_order > sorted_windows[j + 1]->z_order) {
                window_t *temp = sorted_windows[j];
                sorted_windows[j] = sorted_windows[j + 1];
                sorted_windows[j + 1] = temp;
            }
        }
    }
    
    // Render windows back to front
    for (int i = 0; i < window_count; i++) {
        wm_render_window(sorted_windows[i]);
    }
}

void wm_handle_mouse_click(int x, int y, bool left_button) {
    if (!left_button) return;
    
    // Check windows from front to back
    window_t *clicked_window = NULL;
    uint32_t highest_z = 0;
    
    for (int i = 0; i < MAX_WINDOWS; i++) {
        window_t *win = &windows[i];
        if (win->id != 0 && (win->flags & WINDOW_FLAG_VISIBLE) && 
            win->state != WINDOW_STATE_MINIMIZED) {
            
            if (x >= (int)win->x && x < (int)(win->x + win->width) &&
                y >= (int)win->y && y < (int)(win->y + win->height)) {
                
                if (win->z_order > highest_z) {
                    highest_z = win->z_order;
                    clicked_window = win;
                }
            }
        }
    }
    
    if (clicked_window) {
        // Check if close button was clicked
        if ((clicked_window->flags & WINDOW_FLAG_CLOSABLE) &&
            x >= (int)(clicked_window->x + clicked_window->width - 25) &&
            x < (int)(clicked_window->x + clicked_window->width - 5) &&
            y >= (int)(clicked_window->y + 5) &&
            y < (int)(clicked_window->y + 25)) {
            
            wm_destroy_window(clicked_window->id);
            return;
        }
        
        // Focus the clicked window
        wm_focus_window(clicked_window->id);
        
        // TODO: Handle window dragging if clicked on title bar
    }
}

// Desktop implementation
void desktop_init(void) {
    desktop_initialized = true;
    printk("[Desktop] Desktop environment initialized\n");
}

void desktop_render_taskbar(void) {
    struct fb_info fb_info;
    fb_get_info(&fb_info);
    
    uint32_t taskbar_y = fb_info.height - TASKBAR_HEIGHT;
    
    // Draw taskbar background
    fb_draw_rect(0, taskbar_y, fb_info.width, TASKBAR_HEIGHT, TASKBAR_COLOR);
    
    // Draw start button
    fb_draw_rect(5, taskbar_y + 5, START_BUTTON_WIDTH, TASKBAR_HEIGHT - 10, 0x0078D4);
    fb_draw_string(15, taskbar_y + 15, "ParadoxOS", TASKBAR_TEXT_COLOR, 0x0078D4);
    
    // Draw window buttons for open windows
    uint32_t button_x = START_BUTTON_WIDTH + 20;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id != 0 && (windows[i].flags & WINDOW_FLAG_VISIBLE)) {
            window_t *win = &windows[i];
            uint32_t button_color = (win->flags & WINDOW_FLAG_FOCUSED) ? 0x404040 : 0x2D2D30;
            
            fb_draw_rect(button_x, taskbar_y + 5, 150, TASKBAR_HEIGHT - 10, button_color);
            
            // Truncate title if too long
            char truncated_title[20];
            strncpy(truncated_title, win->title, 19);
            truncated_title[19] = '\0';
            
            fb_draw_string(button_x + 5, taskbar_y + 15, truncated_title, TASKBAR_TEXT_COLOR, button_color);
            
            button_x += 155;
            if (button_x > fb_info.width - 155) break;
        }
    }
    
    // Draw clock (simple)
    fb_draw_string(fb_info.width - 100, taskbar_y + 15, "12:00", TASKBAR_TEXT_COLOR, TASKBAR_COLOR);
}

void desktop_render(void) {
    if (!desktop_initialized) return;
    
    struct fb_info fb_info;
    fb_get_info(&fb_info);
    
    // Clear desktop
    fb_clear(desktop_color);
    
    // Draw wallpaper text
    fb_draw_string(50, 50, wallpaper_text, COLOR_WHITE, desktop_color);
    
    // Render all windows
    wm_render_all();
    
    // Render taskbar
    desktop_render_taskbar();
    
    // Draw mouse cursor
    int mouse_x, mouse_y;
    bool left, right, middle;
    get_mouse_state(&mouse_x, &mouse_y, &left, &right, &middle);
    
    // Simple cursor (white arrow)
    fb_draw_line(mouse_x, mouse_y, mouse_x, mouse_y + 10, COLOR_WHITE);
    fb_draw_line(mouse_x, mouse_y, mouse_x + 7, mouse_y + 7, COLOR_WHITE);
    fb_draw_line(mouse_x, mouse_y + 10, mouse_x + 4, mouse_y + 7, COLOR_WHITE);
    
    // Swap buffers if double buffering is enabled
    fb_swap_buffers();
}

void desktop_handle_mouse(int x, int y, bool left, bool right, bool middle) {
    (void)right; (void)middle; // Unused for now
    
    struct fb_info fb_info;
    fb_get_info(&fb_info);
    
    // Check if click is on taskbar
    if (y >= (int)(fb_info.height - TASKBAR_HEIGHT)) {
        // Handle taskbar clicks
        if (left && x >= 5 && x < START_BUTTON_WIDTH + 5) {
            // Start button clicked
            printk("[Desktop] Start button clicked\n");
            // TODO: Show start menu
        }
        return;
    }
    
    // Handle window clicks
    wm_handle_mouse_click(x, y, left);
}

void desktop_handle_keyboard(char key) {
    // Handle global keyboard shortcuts
    if (is_ctrl_pressed()) {
        switch (key) {
            case 'n':
                // Create new window
                wm_create_window(100 + (next_window_id * 20), 100 + (next_window_id * 20), 
                               400, 300, "New Window", 
                               WINDOW_FLAG_RESIZABLE | WINDOW_FLAG_CLOSABLE | 
                               WINDOW_FLAG_MINIMIZABLE | WINDOW_FLAG_MAXIMIZABLE);
                break;
            case 'q':
                // Close focused window
                if (focused_window != 0) {
                    wm_destroy_window(focused_window);
                }
                break;
        }
    }
    
    // Send key to focused window
    // TODO: Implement window input handling
}