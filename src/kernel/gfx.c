#include "gfx.h"

static framebuffer_t back_buffer;
static framebuffer_t front_buffer;
static uint32_t buffer_data[1280 * 800]; // SAFE MODE: Small buffer

void gfx_init(struct limine_framebuffer *fb) {
    front_buffer.address = (uint32_t *)fb->address;
    front_buffer.width = fb->width;
    front_buffer.height = fb->height;
    front_buffer.pitch = fb->pitch;

    back_buffer.address = buffer_data;
    // CLAMPING: Force internal resolution to max 1280x800 to prevent overflow
    back_buffer.width = (fb->width > 1280) ? 1280 : fb->width;
    back_buffer.height = (fb->height > 800) ? 800 : fb->height;
    back_buffer.pitch = back_buffer.width * 4;
}

void gfx_put_pixel(uint32_t x, uint32_t y, color_t color) {
    if (x >= back_buffer.width || y >= back_buffer.height) return;
    back_buffer.address[y * (back_buffer.width) + x] = color;
}

void gfx_blend_pixel(uint32_t x, uint32_t y, color_t color, uint8_t alpha) {
    if (x >= back_buffer.width || y >= back_buffer.height) return;
    
    uint32_t idx = y * (back_buffer.width) + x;
    color_t bg = back_buffer.address[idx];

    uint8_t r_bg = (bg >> 16) & 0xFF;
    uint8_t g_bg = (bg >> 8) & 0xFF;
    uint8_t b_bg = bg & 0xFF;

    uint8_t r_fg = (color >> 16) & 0xFF;
    uint8_t g_fg = (color >> 8) & 0xFF;
    uint8_t b_fg = color & 0xFF;

    uint8_t r = ((r_fg * alpha) + (r_bg * (255 - alpha))) / 255;
    uint8_t g = ((g_fg * alpha) + (g_bg * (255 - alpha))) / 255;
    uint8_t b = ((b_fg * alpha) + (b_bg * (255 - alpha))) / 255;

    back_buffer.address[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void gfx_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            gfx_put_pixel(x + j, y + i, color);
        }
    }
}

void gfx_draw_rect_alpha(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color, uint8_t alpha) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            gfx_blend_pixel(x + j, y + i, color, alpha);
        }
    }
}

void gfx_draw_rounded_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r, color_t color) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            int dx = 0, dy = 0;
            int is_corner = 0;

            if (j < r && i < r) { dx = r - j; dy = r - i; is_corner = 1; }
            else if (j > w - r - 1 && i < r) { dx = j - (w - r - 1); dy = r - i; is_corner = 1; }
            else if (j < r && i > h - r - 1) { dx = r - j; dy = i - (h - r - 1); is_corner = 1; }
            else if (j > w - r - 1 && i > h - r - 1) { dx = j - (w - r - 1); dy = i - (h - r - 1); is_corner = 1; }

            if (is_corner) {
                if ((uint32_t)(dx * dx + dy * dy) <= r * r) gfx_put_pixel(x + j, y + i, color);
            } else {
                gfx_put_pixel(x + j, y + i, color);
            }
        }
    }
}

void gfx_draw_gradient(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t c1, color_t c2) {
    for (uint32_t i = 0; i < h; i++) {
        /* Integer interpolation: color = c1 + (c2 - c1) * i / h */
        uint8_t r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
        uint8_t r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
        
        uint8_t r = r1 + ((int)r2 - r1) * i / h;
        uint8_t g = g1 + ((int)g2 - g1) * i / h;
        uint8_t b = b1 + ((int)b2 - b1) * i / h;
        
        uint32_t row_color = (0xFF << 24) | (r << 16) | (g << 8) | b;
        
        for (uint32_t j = 0; j < w; j++) {
            gfx_put_pixel(x + j, y + i, row_color);
        }
    }
}

void gfx_draw_image(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint32_t* data) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            uint32_t color = data[i * w + j];
            uint8_t alpha = (color >> 24) & 0xFF;
            if (alpha == 255) {
                gfx_put_pixel(x + j, y + i, color);
            } else if (alpha > 0) {
                gfx_blend_pixel(x + j, y + i, color, alpha);
            }
        }
    }
}

void gfx_clear(color_t color) {
    for (uint32_t i = 0; i < back_buffer.width * back_buffer.height; i++) {
        back_buffer.address[i] = color;
    }
}

void gfx_swap_buffers() {
    /* Copy backbuffer to frontbuffer (Clamped Region Only) */
    for (uint32_t i = 0; i < back_buffer.height; i++) {
        for (uint32_t j = 0; j < back_buffer.width; j++) {
            front_buffer.address[i * (front_buffer.pitch / 4) + j] = back_buffer.address[i * (back_buffer.width) + j];
        }
    }
}
