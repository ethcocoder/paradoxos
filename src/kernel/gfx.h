#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stddef.h>
#include "boot/stivale2.h"

struct GFX_Struct {
    uint64_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint16_t fb_bpp;
};

extern struct GFX_Struct gfx;

void framebuffer_init(struct stivale2_struct *stivale2_struct, uint32_t background_color);
void framebuffer_draw_pixel(int x, int y, uint32_t color);
void framebuffer_clear(uint32_t color);
void font_draw_char(char c, int x, int y, uint32_t color);
void font_draw_string(const char *str, int x, int y, uint32_t color);

#endif
