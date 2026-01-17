#ifndef _APP_FRAMEWORK_H
#define _APP_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <gui/window.h>

// Application structure
typedef struct application {
    uint32_t app_id;
    char name[64];
    char version[16];
    uint32_t main_window_id;
    bool running;
    void (*init)(struct application *app);
    void (*update)(struct application *app);
    void (*render)(struct application *app);
    void (*cleanup)(struct application *app);
    void (*handle_event)(struct application *app, uint32_t event_type, void *event_data);
    void *private_data;
} application_t;

// Event types
#define APP_EVENT_KEY_PRESS     1
#define APP_EVENT_KEY_RELEASE   2
#define APP_EVENT_MOUSE_CLICK   3
#define APP_EVENT_MOUSE_MOVE    4
#define APP_EVENT_WINDOW_CLOSE  5
#define APP_EVENT_WINDOW_RESIZE 6

// Event structures
struct app_key_event {
    char key;
    bool shift;
    bool ctrl;
    bool alt;
};

struct app_mouse_event {
    int x, y;
    bool left, right, middle;
};

struct app_window_event {
    uint32_t window_id;
    uint32_t width, height;
};

// Application framework functions
int app_framework_init(void);
uint32_t app_register(application_t *app);
void app_unregister(uint32_t app_id);
void app_run_all(void);
application_t *app_get(uint32_t app_id);

// Built-in applications
void app_text_editor_init(void);
void app_calculator_init(void);
void app_file_manager_init(void);
void app_system_info_init(void);

// Utility functions for applications
uint32_t app_create_window(application_t *app, const char *title, uint32_t width, uint32_t height);
void app_draw_text(uint32_t window_id, uint32_t x, uint32_t y, const char *text, uint32_t color);
void app_draw_button(uint32_t window_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, 
                    const char *text, uint32_t bg_color, uint32_t text_color);

#endif