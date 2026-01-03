#include <stddef.h>
#include <stdint.h>
#include "../boot/limine.h"
#include "gfx.h"
#include "font.h"
#include "cpu.h"
#include "keyboard.h"
#include "mouse.h"
#include "user.h"
#include "vfs.h"
#include "ramdisk.h"
// #include "splash_img.h" // Disabled for stability
// #include "login_img.h"

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
static system_state_t sys_state = SYS_STATE_LOGIN;

void draw_logo(uint32_t x, uint32_t y) {
    // Stylized "P" with geometric hits
    gfx_draw_rect(x, y, 10, 60, COLOR_PURPLE); // Vertical bar
    gfx_draw_rounded_rect(x + 10, y, 40, 35, 15, COLOR_PURPLE); // Curve
    gfx_draw_rect(x + 15, y + 5, 20, 25, 0xFF111111); // Hole in P
}

static char input_buffer[MAX_NAME_LEN] = {0};
static char pass_buffer[MAX_NAME_LEN] = {0};
static int input_ptr = 0;
static int pass_ptr = 0;
static int input_focus = 0; // 0 = Username, 1 = Password

void draw_splash_screen(uint32_t screen_w, uint32_t screen_h) {
    // Optimization: Draw procedural splash instead of large image
    gfx_clear(0);
    draw_logo(screen_w / 2 - 25, screen_h / 2 - 60);

    // Pulse text
    if (((uint32_t)(__builtin_ia32_rdtsc() / 200000000) % 2) == 0) {
        font_draw_string("Press ANY KEY to Start", screen_w / 2 - 80, screen_h - 100, COLOR_WHITE);
    }
}

void draw_kali_login(uint32_t screen_w, uint32_t screen_h, const char* title) {
    // Procedural Background (Restored & Stable)
    gfx_draw_gradient(0, 0, screen_w, screen_h, 0xFF050510, 0xFF101025);
    
    // Central Box (Kali Style)
    uint32_t w = 360, h = 420;
    uint32_t x = (screen_w - w) / 2;
    uint32_t y = (screen_h - h) / 2;
    
    // Shadow & Glass Body
    gfx_draw_rect_alpha(x + 10, y + 10, w, h, 0x000000, 150);
    gfx_draw_rect_alpha(x, y, w, h, 0x111111, 230); // Transparent black
    gfx_draw_rect(x, y, w, 2, COLOR_PURPLE); // Accent top
    
    draw_logo(x + 155, y + 40);
    font_draw_string("PARADOX OS", x + 135, y + 110, COLOR_WHITE);
    font_draw_string(title, x + (w - (k_strlen(title) * 8)) / 2, y + 130, 0xFFAAAAAA);
    
    // Username
    font_draw_string("Username", x + 40, y + 180, 0xFFAAAAAA);
    gfx_draw_rect_alpha(x + 40, y + 200, 280, 40, 0x000000, 180);
    font_draw_string(input_buffer, x + 50, y + 212, COLOR_WHITE);
    if (input_focus == 0 && ((uint32_t)(__builtin_ia32_rdtsc() / 150000000) % 2 == 0))
        gfx_draw_rect(x + 50 + (input_ptr * 8), y + 212, 2, 16, COLOR_WHITE);

    // Password
    font_draw_string("Password", x + 40, y + 250, 0xFFAAAAAA);
    gfx_draw_rect_alpha(x + 40, y + 270, 280, 40, 0x000000, 180);
    char stars[MAX_NAME_LEN] = {0};
    for(int i=0; i<pass_ptr; i++) stars[i] = '*';
    font_draw_string(stars, x + 50, y + 282, COLOR_WHITE);
    if (input_focus == 1 && ((uint32_t)(__builtin_ia32_rdtsc() / 150000000) % 2 == 0))
        gfx_draw_rect(x + 50 + (pass_ptr * 8), y + 282, 2, 16, COLOR_WHITE);

    font_draw_string("TAB: Switch | ENTER: Login", x + 60, y + 340, 0xFF666666);
    if (sys_state == SYS_STATE_LOGIN)
        font_draw_string("Press 'R' to Register", x + 100, y + 370, COLOR_PURPLE);
}

void draw_desktop_icons() {
    struct { char* name; int x, y; color_t color; } icons[] = {
        {"Users",     50, 50,  0xFF00D4FF},
        {"Devices",   50, 150, 0xFF70FF70},
        {"Network",   50, 250, 0xFFFF7070},
        {"Terminal",  50, 350, 0xFFFFFF70},
        {"Explorer", 150, 50,  0xFFFFA500}
    };
    for (int i = 0; i < 5; i++) {
        gfx_draw_rounded_rect(icons[i].x, icons[i].y, 50, 50, 10, icons[i].color);
        gfx_draw_rect_alpha(icons[i].x + 10, icons[i].y + 10, 30, 30, 0xFFFFFFFF, 50);
        font_draw_string(icons[i].name, icons[i].x - 10, icons[i].y + 60, COLOR_WHITE);
    }
}

void _start(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    gfx_init(framebuffer);
    
    // Immediate Visual Feedback
    gfx_clear(0);
    draw_logo(framebuffer->width / 2 - 25, framebuffer->height / 2 - 60);
    font_draw_string("Paradox Kernel Loading...", framebuffer->width / 2 - 80, framebuffer->height - 50, COLOR_PURPLE);
    gfx_swap_buffers();

    cpu_init();
    keyboard_init();
    mouse_init();
    user_init();
    fs_root = ramdisk_init();

    static int in_splash = 1;
    static int splash_counter = 0;

    // Finally enable interrupts just before loop
    cpu_enable_interrupts();

    for (;;) {
        mouse_state_t* m = mouse_get_state();
        char key = keyboard_get_last_key();

        // Trail Logic
        trail_x[trail_ptr] = m->x;
        trail_y[trail_ptr] = m->y;
        trail_ptr = (trail_ptr + 1) % MAX_TRAILS;

        if (in_splash) {
            draw_splash_screen(framebuffer->width, framebuffer->height);
            splash_counter++;
            // Auto-transition after approx 200 frames (longer delay for logo visibility)
            if (splash_counter > 200) in_splash = 0; 
            gfx_swap_buffers();
            continue;
        }

        /* Input Handling */
        if (sys_state == SYS_STATE_LOGIN || sys_state == SYS_STATE_REGISTER) {
            if (key == '\t') input_focus = !input_focus;
            else if (key == '\b') {
                if (input_focus == 0 && input_ptr > 0) input_buffer[--input_ptr] = 0;
                else if (input_focus == 1 && pass_ptr > 0) pass_buffer[--pass_ptr] = 0;
            }
            else if (key == '\n') {
                if (sys_state == SYS_STATE_LOGIN) {
                    if (user_login(input_buffer, pass_buffer)) sys_state = SYS_STATE_DESKTOP;
                } else {
                    if (user_register(input_buffer, pass_buffer)) sys_state = SYS_STATE_LOGIN;
                }
                for(int i=0; i<MAX_NAME_LEN; i++) input_buffer[i] = pass_buffer[i] = 0;
                input_ptr = pass_ptr = 0;
            }
            else if (key == 'r' && sys_state == SYS_STATE_LOGIN) {
                sys_state = SYS_STATE_REGISTER;
                input_ptr = pass_ptr = 0;
            }
            else if (key != 0 && key >= 32 && key <= 126) {
                if (input_focus == 0 && input_ptr < MAX_NAME_LEN - 1) input_buffer[input_ptr++] = key;
                else if (input_focus == 1 && pass_ptr < MAX_NAME_LEN - 1) pass_buffer[pass_ptr++] = key;
            }
        }

        if (sys_state == SYS_STATE_LOGIN) {
            draw_kali_login(framebuffer->width, framebuffer->height, "Paradox Login");
        } else if (sys_state == SYS_STATE_REGISTER) {
            draw_kali_login(framebuffer->width, framebuffer->height, "User Registration");
        } else {
            /* Desktop Logic */
            static window_t main_win;
            static int win_init = 0;
            if (!win_init) {
                main_win.x = (framebuffer->width - 700) / 2;
                main_win.y = (framebuffer->height - 500) / 2;
                main_win.w = 700; main_win.h = 500;
                win_init = 1;
            }
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

            // Explorer File Listing
            font_draw_string("Directory: /ramdisk/", main_win.x + 20, main_win.y + 50, 0xFFAAAAAA);
            gfx_draw_rect(main_win.x + 20, main_win.y + 70, main_win.w - 40, 1, 0xFF444444);
            
            struct dirent *de;
            int file_idx = 0;
            while ((de = vfs_readdir(fs_root, file_idx)) != 0) {
                gfx_draw_rect(main_win.x + 30, main_win.y + 85 + (file_idx * 30), 20, 20, COLOR_PURPLE);
                font_draw_string(de->name, main_win.x + 60, main_win.y + 87 + (file_idx * 30), COLOR_WHITE);
                file_idx++;
            }
            
            font_draw_string("VFS initialized. System stable.", main_win.x + 20, main_win.y + main_win.h - 30, 0xFF666666);
            
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



