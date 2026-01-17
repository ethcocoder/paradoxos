#include <app/framework.h>
#include <gui/window.h>
#include <drivers/framebuffer.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <mm/pmm.h>

#define MAX_APPLICATIONS 32

static application_t *applications[MAX_APPLICATIONS];
static uint32_t next_app_id = 1;
static bool framework_initialized = false;

int app_framework_init(void) {
    memset(applications, 0, sizeof(applications));
    next_app_id = 1;
    framework_initialized = true;
    
    // Initialize built-in applications
    app_text_editor_init();
    app_calculator_init();
    app_file_manager_init();
    app_system_info_init();
    
    printk("[AppFramework] Application framework initialized\n");
    return 0;
}

uint32_t app_register(application_t *app) {
    if (!framework_initialized || !app) return 0;
    
    for (int i = 0; i < MAX_APPLICATIONS; i++) {
        if (applications[i] == NULL) {
            app->app_id = next_app_id++;
            applications[i] = app;
            
            if (app->init) {
                app->init(app);
            }
            
            printk("[AppFramework] Registered application '%s' with ID %u\n", app->name, app->app_id);
            return app->app_id;
        }
    }
    
    return 0;
}

void app_unregister(uint32_t app_id) {
    for (int i = 0; i < MAX_APPLICATIONS; i++) {
        if (applications[i] && applications[i]->app_id == app_id) {
            application_t *app = applications[i];
            
            if (app->cleanup) {
                app->cleanup(app);
            }
            
            if (app->main_window_id) {
                wm_destroy_window(app->main_window_id);
            }
            
            printk("[AppFramework] Unregistered application '%s'\n", app->name);
            applications[i] = NULL;
            break;
        }
    }
}

void app_run_all(void) {
    if (!framework_initialized) return;
    
    for (int i = 0; i < MAX_APPLICATIONS; i++) {
        if (applications[i] && applications[i]->running) {
            application_t *app = applications[i];
            
            if (app->update) {
                app->update(app);
            }
            
            if (app->render) {
                app->render(app);
            }
        }
    }
}

application_t *app_get(uint32_t app_id) {
    for (int i = 0; i < MAX_APPLICATIONS; i++) {
        if (applications[i] && applications[i]->app_id == app_id) {
            return applications[i];
        }
    }
    return NULL;
}

uint32_t app_create_window(application_t *app, const char *title, uint32_t width, uint32_t height) {
    if (!app) return 0;
    
    uint32_t window_id = wm_create_window(100 + (app->app_id * 30), 100 + (app->app_id * 30), 
                                         width, height, title,
                                         WINDOW_FLAG_RESIZABLE | WINDOW_FLAG_CLOSABLE | 
                                         WINDOW_FLAG_MINIMIZABLE | WINDOW_FLAG_MAXIMIZABLE);
    
    if (!app->main_window_id) {
        app->main_window_id = window_id;
    }
    
    return window_id;
}

void app_draw_text(uint32_t window_id, uint32_t x, uint32_t y, const char *text, uint32_t color) {
    window_t *window = wm_get_window(window_id);
    if (!window || !window->buffer || !text) return;
    
    // Draw text to window buffer
    uint32_t cur_x = x;
    while (*text) {
        if (*text == '\n') {
            cur_x = x;
            y += 16;
        } else {
            // Simple character rendering to window buffer
            for (int row = 0; row < 16; row++) {
                for (int col = 0; col < 8; col++) {
                    if (cur_x + col < window->width && y + row < window->height - 30) {
                        // Simple character pattern (just a placeholder)
                        if ((row > 2 && row < 14) && (col > 1 && col < 7)) {
                            window->buffer[(y + row - 30) * window->width + (cur_x + col)] = color;
                        }
                    }
                }
            }
            cur_x += 8;
        }
        text++;
    }
    
    window->needs_redraw = true;
}

void app_draw_button(uint32_t window_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, 
                    const char *text, uint32_t bg_color, uint32_t text_color) {
    window_t *window = wm_get_window(window_id);
    if (!window || !window->buffer) return;
    
    // Draw button background
    for (uint32_t row = 0; row < height; row++) {
        for (uint32_t col = 0; col < width; col++) {
            if (x + col < window->width && y + row < window->height - 30) {
                window->buffer[(y + row - 30) * window->width + (x + col)] = bg_color;
            }
        }
    }
    
    // Draw button border
    for (uint32_t col = 0; col < width; col++) {
        if (x + col < window->width) {
            if (y < window->height - 30) {
                window->buffer[(y - 30) * window->width + (x + col)] = 0x808080; // Top
            }
            if (y + height - 1 < window->height - 30) {
                window->buffer[(y + height - 1 - 30) * window->width + (x + col)] = 0x808080; // Bottom
            }
        }
    }
    for (uint32_t row = 0; row < height; row++) {
        if (y + row < window->height - 30) {
            if (x < window->width) {
                window->buffer[(y + row - 30) * window->width + x] = 0x808080; // Left
            }
            if (x + width - 1 < window->width) {
                window->buffer[(y + row - 30) * window->width + (x + width - 1)] = 0x808080; // Right
            }
        }
    }
    
    // Draw button text (centered)
    if (text) {
        uint32_t text_x = x + (width - strlen(text) * 8) / 2;
        uint32_t text_y = y + (height - 16) / 2;
        app_draw_text(window_id, text_x, text_y, text, text_color);
    }
    
    window->needs_redraw = true;
}

// Built-in applications

// Text Editor Application
static application_t text_editor_app;

static void text_editor_init(application_t *app) {
    app->main_window_id = app_create_window(app, "ParadoxOS Text Editor", 600, 400);
    app->running = true;
    
    // Draw initial content
    app_draw_text(app->main_window_id, 10, 40, "Welcome to ParadoxOS Text Editor!", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 60, "This is a simple text editor.", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 80, "Type to edit text...", COLOR_GRAY);
}

static void text_editor_update(application_t *app) {
    // Handle text editor logic
    (void)app;
}

static void text_editor_render(application_t *app) {
    // Render text editor
    (void)app;
}

void app_text_editor_init(void) {
    memset(&text_editor_app, 0, sizeof(text_editor_app));
    strcpy(text_editor_app.name, "Text Editor");
    strcpy(text_editor_app.version, "1.0");
    text_editor_app.init = text_editor_init;
    text_editor_app.update = text_editor_update;
    text_editor_app.render = text_editor_render;
    
    app_register(&text_editor_app);
}

// Calculator Application
static application_t calculator_app;

static void calculator_init(application_t *app) {
    app->main_window_id = app_create_window(app, "ParadoxOS Calculator", 300, 400);
    app->running = true;
    
    // Draw calculator interface
    app_draw_text(app->main_window_id, 10, 40, "Calculator", COLOR_BLACK);
    
    // Draw number buttons
    const char *buttons[] = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "0", ".", "=", "+"};
    for (int i = 0; i < 16; i++) {
        int row = i / 4;
        int col = i % 4;
        app_draw_button(app->main_window_id, 20 + col * 60, 80 + row * 50, 50, 40, 
                       buttons[i], COLOR_LIGHT_GRAY, COLOR_BLACK);
    }
}

static void calculator_update(application_t *app) {
    (void)app;
}

static void calculator_render(application_t *app) {
    (void)app;
}

void app_calculator_init(void) {
    memset(&calculator_app, 0, sizeof(calculator_app));
    strcpy(calculator_app.name, "Calculator");
    strcpy(calculator_app.version, "1.0");
    calculator_app.init = calculator_init;
    calculator_app.update = calculator_update;
    calculator_app.render = calculator_render;
    
    app_register(&calculator_app);
}

// File Manager Application
static application_t file_manager_app;

static void file_manager_init(application_t *app) {
    app->main_window_id = app_create_window(app, "ParadoxOS File Manager", 500, 350);
    app->running = true;
    
    // Draw file manager interface
    app_draw_text(app->main_window_id, 10, 40, "File Manager - /", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 70, "README.txt", COLOR_BLUE);
    app_draw_text(app->main_window_id, 10, 90, "home/", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 110, "tmp/", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 130, "dev/", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 150, "proc/", COLOR_BLACK);
}

void app_file_manager_init(void) {
    memset(&file_manager_app, 0, sizeof(file_manager_app));
    strcpy(file_manager_app.name, "File Manager");
    strcpy(file_manager_app.version, "1.0");
    file_manager_app.init = file_manager_init;
    
    app_register(&file_manager_app);
}

// System Information Application
static application_t system_info_app;

static void system_info_init(application_t *app) {
    app->main_window_id = app_create_window(app, "ParadoxOS System Information", 450, 300);
    app->running = true;
    
    // Draw system information
    app_draw_text(app->main_window_id, 10, 40, "ParadoxOS System Information", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 70, "OS: ParadoxOS v1.0", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 90, "Kernel: Hybrid Kernel", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 110, "Architecture: x86_64", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 130, "Memory: 256 MB", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 150, "Graphics: Framebuffer", COLOR_BLACK);
    app_draw_text(app->main_window_id, 10, 170, "File System: VFS", COLOR_BLACK);
}

void app_system_info_init(void) {
    memset(&system_info_app, 0, sizeof(system_info_app));
    strcpy(system_info_app.name, "System Info");
    strcpy(system_info_app.version, "1.0");
    system_info_app.init = system_info_init;
    
    app_register(&system_info_app);
}