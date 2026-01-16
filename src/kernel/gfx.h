#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stddef.h>
#include "../boot/limine.h"

typedef uint32_t color_t;

#define COLOR_PURPLE 0xFF8000FF
#define COLOR_DARK   0xFF1A1A1A
#define COLOR_WHITE  0xFFFFFFFF
#define COLOR_ACCENT 0xFF00D4FF

typedef struct {
    uint32_t *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
} framebuffer_t;

void gfx_init(struct limine_framebuffer *fb);
void gfx_put_pixel(uint32_t x, uint32_t y, color_t color);
void gfx_blend_pixel(uint32_t x, uint32_t y, color_t color, uint8_t alpha);
void gfx_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
void gfx_draw_rect_alpha(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color, uint8_t alpha);
void gfx_draw_rounded_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r, color_t color);
void gfx_draw_gradient(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t c1, color_t c2);
void gfx_clear(color_t color);

/* Double buffering support */
void gfx_swap_buffers();

#endif
