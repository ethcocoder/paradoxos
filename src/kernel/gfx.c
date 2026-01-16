#include "gfx.h"

static framebuffer_t back_buffer;
static framebuffer_t front_buffer;
static uint32_t buffer_data[1920 * 1080]; // Temporary backbuffer (Static for now to avoid complexity)

void gfx_init(struct limine_framebuffer *fb) {
    front_buffer.address = (uint32_t *)fb->address;
    front_buffer.width = fb->width;
    front_buffer.height = fb->height;
    front_buffer.pitch = fb->pitch;

    back_buffer.address = buffer_data;
    back_buffer.width = fb->width;
    back_buffer.height = fb->height;
    back_buffer.pitch = fb->width * 4;
}

void gfx_put_pixel(uint32_t x, uint32_t y, color_t color) {
    if (x >= back_buffer.width || y >= back_buffer.height) return;
    back_buffer.address[y * (back_buffer.pitch / 4) + x] = color;
}

void gfx_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            gfx_put_pixel(x + j, y + i, color);
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


void gfx_clear(color_t color) {
    for (uint32_t i = 0; i < back_buffer.width * back_buffer.height; i++) {
        back_buffer.address[i] = color;
    }
}

void gfx_swap_buffers() {
    /* Copy backbuffer to frontbuffer */
    for (uint32_t i = 0; i < front_buffer.height; i++) {
        for (uint32_t j = 0; j < front_buffer.width; j++) {
            front_buffer.address[i * (front_buffer.pitch / 4) + j] = back_buffer.address[i * (back_buffer.width) + j];
        }
    }
}
