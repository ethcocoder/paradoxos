#include "gfx.h"
#include "font.h" 
#include "boot/stivale2.h"
#include <stddef.h>

struct GFX_Struct gfx;

extern void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id);

void framebuffer_init(struct stivale2_struct *stivale2_struct, uint32_t background_color) {
    struct stivale2_struct_tag_framebuffer *fb_info = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    if (fb_info) {
        gfx.fb_addr   = fb_info->framebuffer_addr;
        gfx.fb_width  = fb_info->framebuffer_width;
        gfx.fb_height = fb_info->framebuffer_height;
        gfx.fb_pitch  = fb_info->framebuffer_pitch;
        gfx.fb_bpp    = fb_info->framebuffer_bpp;
    }

    framebuffer_clear(background_color);
}

void framebuffer_draw_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)gfx.fb_width || y < 0 || y >= (int)gfx.fb_height) return;
    uint32_t *fb = (uint32_t *)gfx.fb_addr;
    fb[y * (gfx.fb_pitch / 4) + x] = color;
}

void framebuffer_clear(uint32_t color) {
    uint32_t *fb = (uint32_t *)gfx.fb_addr;
    for (uint32_t i = 0; i < gfx.fb_width * gfx.fb_height; i++) {
        fb[i] = color;
    }
}
