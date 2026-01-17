#ifndef _DRIVERS_FRAMEBUFFER_H
#define _DRIVERS_FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Framebuffer info structure
struct fb_info {
    uint64_t width;
    uint64_t height;
    uint16_t bpp;
    uint64_t pitch;
    uint64_t address;
    bool double_buffering;
};

// Basic framebuffer functions
void fb_init(void);
void fb_get_info(struct fb_info *info);
uint32_t *fb_get_current_buffer(void);
void fb_swap_buffers(void);

// Pixel operations
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_put_pixel_alpha(uint32_t x, uint32_t y, uint32_t color, uint8_t alpha);
void fb_clear(uint32_t color);

// Shape drawing
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void fb_draw_rect_alpha(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, uint8_t alpha);
void fb_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
void fb_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color);

// Text rendering
void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg_color, uint32_t bg_color);
void fb_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg_color, uint32_t bg_color);

// Advanced operations
void fb_copy_region(uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y, 
                   uint32_t width, uint32_t height);
void fb_fill_gradient(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                     uint32_t color1, uint32_t color2, bool horizontal);

// Color constants
#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF
#define COLOR_RED       0xFF0000
#define COLOR_GREEN     0x00FF00
#define COLOR_BLUE      0x0000FF
#define COLOR_YELLOW    0xFFFF00
#define COLOR_CYAN      0x00FFFF
#define COLOR_MAGENTA   0xFF00FF
#define COLOR_GRAY      0x808080
#define COLOR_DARK_GRAY 0x404040
#define COLOR_LIGHT_GRAY 0xC0C0C0

#endif
