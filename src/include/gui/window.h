#ifndef _GUI_WINDOW_H
#define _GUI_WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_WINDOWS 64
#define WINDOW_TITLE_MAX 256

// Window states
typedef enum {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_CLOSED
} window_state_t;

// Window flags
#define WINDOW_FLAG_RESIZABLE   (1 << 0)
#define WINDOW_FLAG_CLOSABLE    (1 << 1)
#define WINDOW_FLAG_MINIMIZABLE (1 << 2)
#define WINDOW_FLAG_MAXIMIZABLE (1 << 3)
#define WINDOW_FLAG_VISIBLE     (1 << 4)
#define WINDOW_FLAG_FOCUSED     (1 << 5)

// Window structure
typedef struct window {
    uint32_t id;
    uint32_t x, y;
    uint32_t width, height;
    uint32_t z_order;
    window_state_t state;
    uint32_t flags;
    char title[WINDOW_TITLE_MAX];
    uint32_t *buffer;
    bool needs_redraw;
    uint32_t owner_pid;
} window_t;

// Window manager functions
void wm_init(void);
uint32_t wm_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, 
                         const char *title, uint32_t flags);
void wm_destroy_window(uint32_t window_id);
void wm_move_window(uint32_t window_id, uint32_t x, uint32_t y);
void wm_resize_window(uint32_t window_id, uint32_t width, uint32_t height);
void wm_set_window_state(uint32_t window_id, window_state_t state);
void wm_focus_window(uint32_t window_id);
void wm_render_all(void);
void wm_handle_mouse_click(int x, int y, bool left_button);
window_t *wm_get_window(uint32_t window_id);

// Desktop functions
void desktop_init(void);
void desktop_render(void);
void desktop_handle_mouse(int x, int y, bool left, bool right, bool middle);
void desktop_handle_keyboard(char key);

#endif