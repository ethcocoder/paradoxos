#include <stddef.h>
#include <stdint.h>
#include "../boot/limine.h"
#include "gfx.h"
#include "font.h"
#include "cpu.h"
#include "keyboard.h"
#include "mouse.h"
#include "user.h"

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

#define MAX_TRAILS 10
static int trail_x[MAX_TRAILS];
static int trail_y[MAX_TRAILS];
static int trail_ptr = 0;

void draw_logo(uint32_t x, uint32_t y) {
    // Stylized "P" with geometric hits
    gfx_draw_rect(x, y, 10, 60, COLOR_PURPLE); // Vertical bar
    gfx_draw_rounded_rect(x + 10, y, 40, 35, 15, COLOR_PURPLE); // Curve
    gfx_draw_rect(x + 15, y + 5, 20, 25, 0xFF111111); // Hole in P
}

void draw_login_screen(uint32_t screen_w, uint32_t screen_h) {
    gfx_draw_gradient(0, 0, screen_w, screen_h, 0xFF050510, 0xFF101025);
    
    // Aurora Flow (Mock: shifting colors or particles)
    for(int i=0; i<5; i++) {
        gfx_draw_rect_alpha(i*200, 100 + i*50, 400, 100, COLOR_ACCENT, 30);
    }

    uint32_t card_w = 400, card_h = 350;
    uint32_t card_x = (screen_w - card_w) / 2;
    uint32_t card_y = (screen_h - card_h) / 2;
    
    // Glass Card
    gfx_draw_rect_alpha(card_x, card_y, card_w, card_h, 0xFF333333, 150);
    gfx_draw_rounded_rect(card_x, card_y, card_w, 2, 0, COLOR_PURPLE);
    
    draw_logo(card_x + 175, card_y + 40);
    font_draw_string("PARADOX OS", card_x + 150, card_y + 110, COLOR_WHITE);
    font_draw_string("Cosmic Awakening", card_x + 140, card_y + 130, 0xFFAAAAAA);

    gfx_draw_rect_alpha(card_x + 160, card_y + 160, 80, 80, 0xFF444444, 100);
    gfx_draw_rect_alpha(card_x + 50, card_y + 260, 300, 40, 0xFF000000, 150);
    font_draw_string("Click to Login", card_x + 140, card_y + 272, 0xFF888888);
}

void draw_desktop_icons() {
    struct { char* name; int x, y; color_t color; } icons[] = {
        {"Users",     50, 50,  0xFF00D4FF},
        {"Docs",      50, 150, 0xFF70FF70},
        {"Music",     50, 250, 0xFFFF7070},
        {"Videos",    50, 350, 0xFFFFFF70},
        {"Pictures", 150, 50,  0xFFFFA500}
    };
    for (int i = 0; i < 5; i++) {
        // Mystical Symbol Icons (Geometric)
        gfx_draw_rounded_rect(icons[i].x, icons[i].y, 50, 50, 10, icons[i].color);
        gfx_draw_rect_alpha(icons[i].x + 10, icons[i].y + 10, 30, 30, 0xFFFFFFFF, 50);
        font_draw_string(icons[i].name, icons[i].x - 10, icons[i].y + 60, COLOR_WHITE);
    }
}

void _start(void) {
    cpu_init();
    keyboard_init();
    mouse_init();
    user_init();

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    gfx_init(framebuffer);

    // Boot Animation: Fade In (Cosmic Awakening)
    for(int alpha=0; alpha<=255; alpha+=15) {
        gfx_clear(0);
        gfx_draw_rect_alpha(framebuffer->width/2 - 50, framebuffer->height/2 - 50, 100, 100, COLOR_PURPLE, alpha);
        font_draw_string("Awakening...", framebuffer->width/2 - 40, framebuffer->height/2 + 60, (alpha << 24) | 0xFFFFFF);
        gfx_swap_buffers();
    }

    window_t main_win = {
        .x = (framebuffer->width - 700) / 2,
        .y = (framebuffer->height - 500) / 2,
        .w = 700, .h = 500, .is_dragging = 0
    };

    for (;;) {
        mouse_state_t* m = mouse_get_state();

        // Trail Logic
        trail_x[trail_ptr] = m->x;
        trail_y[trail_ptr] = m->y;
        trail_ptr = (trail_ptr + 1) % MAX_TRAILS;

        if (sys_state == SYS_STATE_LOGIN) {
            draw_login_screen(framebuffer->width, framebuffer->height);
            if (m->left_button && m->x > (framebuffer->width/2 - 200) && m->x < (framebuffer->width/2 + 200) &&
                m->y > (framebuffer->height/2 + 80)) {
                if (user_login("admin", "1234")) {
                    sys_state = SYS_STATE_DESKTOP;
                }
            }
        } else {
            /* Desktop Logic */
            if (m->left_button) {
                if (!main_win.is_dragging) {
                    if (m->x >= main_win.x && m->x <= main_win.x + main_win.w &&
                        m->y >= main_win.y && m->y <= main_win.y + 30) {
                        main_win.is_dragging = 1;
                        main_win.drag_off_x = m->x - main_win.x;
                        main_win.drag_off_y = m->y - main_win.y;
                    }
                } else {
                    main_win.x = m->x - main_win.drag_off_x;
                    main_win.y = m->y - main_win.drag_off_y;
                }
            } else {
                main_win.is_dragging = 0;
            }

            gfx_draw_gradient(0, 0, framebuffer->width, framebuffer->height, 0xFF0A0A1F, 0xFF1A1A3F);
            draw_desktop_icons();

            // Neo-Glass Window
            gfx_draw_rect_alpha(main_win.x + 8, main_win.y + 8, main_win.w, main_win.h, 0x000000, 100); // Shadow
            gfx_draw_rect_alpha(main_win.x, main_win.y, main_win.w, main_win.h, 0x222222, 180); // Glass Body
            
            color_t title_color = main_win.is_dragging ? COLOR_ACCENT : COLOR_PURPLE;
            gfx_draw_rounded_rect(main_win.x, main_win.y, main_win.w, 30, 5, title_color);
            font_draw_string("Paradox Neo-Glass Explorer", main_win.x + 10, main_win.y + 10, COLOR_WHITE);

            font_draw_string("Welcome, admin", main_win.x + 20, main_win.y + 60, 0xFFAAAAAA);
            font_draw_string("This is the future of intelligence.", main_win.x + 20, main_win.y + 100, COLOR_WHITE);
            
            /* Taskbar */
            uint32_t bar_h = 50;
            gfx_draw_rect_alpha(0, framebuffer->height - bar_h, framebuffer->width, bar_h, 0x111111, 200);
            gfx_draw_rounded_rect(15, framebuffer->height - bar_h + 7, 35, 35, 8, COLOR_PURPLE);
            font_draw_string("P", 27, framebuffer->height - bar_h + 17, COLOR_WHITE);
        }

        /* Mouse Cursor with Trails */
        for(int i=0; i<MAX_TRAILS; i++) {
            int t_idx = (trail_ptr + i) % MAX_TRAILS;
            gfx_draw_rect_alpha(trail_x[t_idx], trail_y[t_idx], 4, 4, COLOR_ACCENT, (i * 255) / MAX_TRAILS);
        }
        gfx_draw_rect(m->x, m->y, 8, 8, COLOR_WHITE);
        
        gfx_swap_buffers();
    }
}



