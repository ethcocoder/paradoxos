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

void _start(void) {
    cpu_init();
    keyboard_init();
    mouse_init();

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    gfx_init(framebuffer);

    for (;;) {
        /* 1. Draw the Background */
        gfx_draw_gradient(0, 0, framebuffer->width, framebuffer->height, 0xFF0A0A1F, 0xFF1A1A3F);

        /* 2. Draw Window */
        uint32_t win_w = 600, win_h = 400;
        uint32_t win_x = (framebuffer->width - win_w) / 2;
        uint32_t win_y = (framebuffer->height - win_h) / 2;
        
        gfx_draw_rect(win_x + 5, win_y + 5, win_w, win_h, 0x88000000);
        gfx_draw_rect(win_x, win_y, win_w, win_h, 0xAA222222);
        gfx_draw_rect(win_x, win_y, win_w, 30, 0xFF333333);
        font_draw_string("Paradox Window Manager", win_x + 10, win_y + 10, COLOR_WHITE);

        font_draw_string("Welcome to Paradox OS", win_x + 100, win_y + 150, COLOR_WHITE);
        font_draw_string("A Next-Generation Intelligent System", win_x + 100, win_y + 170, 0xFFAAAAAA);
        
        gfx_draw_rect(win_x, win_y, win_w, 1, 0xFF444444);
        
        /* 3. Taskbar */
        uint32_t bar_h = 45;
        gfx_draw_rect(0, framebuffer->height - bar_h, framebuffer->width, bar_h, 0xDD111111);
        gfx_draw_rect(10, framebuffer->height - bar_h + 5, 35, 35, COLOR_PURPLE);
        font_draw_string("P", 22, framebuffer->height - bar_h + 15, COLOR_WHITE);

        /* 4. Mouse Cursor (A glowing square for now for visibility) */
        mouse_state_t* m = mouse_get_state();
        gfx_draw_rect(m->x, m->y, 10, 10, 0xFF00D4FF); // Cyan Glow Cursor
        
        /* 5. Flip Buffers */
        gfx_swap_buffers();
    }
}

