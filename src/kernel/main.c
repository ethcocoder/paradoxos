#include <stddef.h>
#include <stdint.h>
#include "../boot/limine.h"
#include "gfx.h"
#include "font.h"
#include "cpu.h"
#include "keyboard.h"
#include "mouse.h"

__attribute__((used, section(".requests")))
volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void hcf(void) {
    for (;;) {
        __asm__("hlt");
    }
}

typedef struct {
    int x, y;
    int w, h;
    int is_dragging;
    int drag_off_x, drag_off_y;
} window_t;

void _start(void) {
    cpu_init();
    keyboard_init();
    mouse_init();

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    gfx_init(framebuffer);

    /* Initialize our first window */
    window_t main_win = {
        .x = (framebuffer->width - 600) / 2,
        .y = (framebuffer->height - 400) / 2,
        .w = 600, .h = 400,
        .is_dragging = 0
    };

    for (;;) {
        mouse_state_t* m = mouse_get_state();

        /* Window Dragging Logic */
        // Title bar is the top 30 pixels
        if (m->left_button) {
            if (!main_win.is_dragging) {
                // Check if mouse is within title bar
                if (m->x >= main_win.x && m->x <= main_win.x + main_win.w &&
                    m->y >= main_win.y && m->y <= main_win.y + 30) {
                    main_win.is_dragging = 1;
                    main_win.drag_off_x = m->x - main_win.x;
                    main_win.drag_off_y = m->y - main_win.y;
                }
            } else {
                // Update position
                main_win.x = m->x - main_win.drag_off_x;
                main_win.y = m->y - main_win.drag_off_y;
            }
        } else {
            main_win.is_dragging = 0;
        }

        /* 1. Draw the Background */
        gfx_draw_gradient(0, 0, framebuffer->width, framebuffer->height, 0xFF0A0A1F, 0xFF1A1A3F);

        /* 2. Draw Window */
        // Shadow
        gfx_draw_rect(main_win.x + 5, main_win.y + 5, main_win.w, main_win.h, 0x88000000);
        // Body
        gfx_draw_rect(main_win.x, main_win.y, main_win.w, main_win.h, 0xAA222222);
        // Title Bar (Highlights if dragging)
        color_t title_color = main_win.is_dragging ? 0xFF444444 : 0xFF333333;
        gfx_draw_rect(main_win.x, main_win.y, main_win.w, 30, title_color);
        font_draw_string("Paradox Window Manager", main_win.x + 10, main_win.y + 10, COLOR_WHITE);

        font_draw_string("Welcome to Paradox OS", main_win.x + 100, main_win.y + 150, COLOR_WHITE);
        font_draw_string("Moving windows with the mouse enabled!", main_win.x + 100, main_win.y + 170, 0xFFAAAAAA);
        
        gfx_draw_rect(main_win.x, main_win.y, main_win.w, 1, 0xFF444444);
        
        /* 3. Taskbar */
        uint32_t bar_h = 45;
        gfx_draw_rect(0, framebuffer->height - bar_h, framebuffer->width, bar_h, 0xDD111111);
        gfx_draw_rect(10, framebuffer->height - bar_h + 5, 35, 35, COLOR_PURPLE);
        font_draw_string("P", 22, framebuffer->height - bar_h + 15, COLOR_WHITE);

        /* 4. Mouse Cursor (A glowing arrow-like shape) */
        gfx_draw_rect(m->x, m->y, 8, 8, 0xFF00D4FF);
        gfx_draw_rect(m->x + 2, m->y + 2, 4, 4, COLOR_WHITE);
        
        /* 5. Flip Buffers */
        gfx_swap_buffers();
    }
}


