#include <drivers/framebuffer.h>
#include <limine.h>
#include <stddef.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <mm/pmm.h>

// Helper function
static inline int abs(int x) {
    return x < 0 ? -x : x;
}

// The Limine request
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static struct limine_framebuffer *fb = NULL;
static uint32_t *back_buffer = NULL;
static bool double_buffering = false;

// Font data (8x16 bitmap font)
static const uint8_t font_8x16[256][16] = {
    // Basic ASCII characters (simplified for demo)
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['A'] = {0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['B'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['C'] = {0x00, 0x00, 0x3C, 0x66, 0x60, 0x60, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['H'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['e'] = {0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x60, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['l'] = {0x00, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['o'] = {0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['P'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['a'] = {0x00, 0x00, 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['r'] = {0x00, 0x00, 0x00, 0x00, 0x6C, 0x76, 0x66, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['d'] = {0x00, 0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['x'] = {0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['O'] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['S'] = {0x00, 0x00, 0x3C, 0x66, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Add more characters as needed...
};

void fb_init(void) {
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        printk("[FB] No framebuffer available\n");
        return;
    }

    fb = framebuffer_request.response->framebuffers[0];
    
    printk("[FB] Framebuffer initialized:\n");
    printk("[FB]   Resolution: %lux%lu\n", fb->width, fb->height);
    printk("[FB]   BPP: %u\n", fb->bpp);
    printk("[FB]   Pitch: %lu\n", fb->pitch);
    printk("[FB]   Address: 0x%p\n", fb->address);
    
    // Allocate back buffer for double buffering
    size_t buffer_size = fb->width * fb->height * (fb->bpp / 8);
    size_t pages_needed = (buffer_size + PAGE_SIZE - 1) / PAGE_SIZE;
    back_buffer = (uint32_t *)pmm_alloc(pages_needed);
    
    if (back_buffer) {
        double_buffering = true;
        printk("[FB] Double buffering enabled\n");
    } else {
        printk("[FB] Warning: Could not allocate back buffer\n");
    }
}

uint32_t *fb_get_current_buffer(void) {
    if (double_buffering && back_buffer) {
        return back_buffer;
    }
    return (uint32_t *)fb->address;
}

void fb_swap_buffers(void) {
    if (!double_buffering || !back_buffer || !fb) return;
    
    // Copy back buffer to front buffer
    size_t buffer_size = fb->width * fb->height * (fb->bpp / 8);
    memcpy((void *)fb->address, back_buffer, buffer_size);
}

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb) return;
    if (x >= fb->width || y >= fb->height) return;

    uint32_t *buffer = fb_get_current_buffer();
    buffer[y * fb->width + x] = color;
}

void fb_put_pixel_alpha(uint32_t x, uint32_t y, uint32_t color, uint8_t alpha) {
    if (!fb) return;
    if (x >= fb->width || y >= fb->height) return;
    
    uint32_t *buffer = fb_get_current_buffer();
    uint32_t existing = buffer[y * fb->width + x];
    
    // Extract RGB components
    uint8_t r_new = (color >> 16) & 0xFF;
    uint8_t g_new = (color >> 8) & 0xFF;
    uint8_t b_new = color & 0xFF;
    
    uint8_t r_old = (existing >> 16) & 0xFF;
    uint8_t g_old = (existing >> 8) & 0xFF;
    uint8_t b_old = existing & 0xFF;
    
    // Alpha blend
    uint8_t r_final = (r_new * alpha + r_old * (255 - alpha)) / 255;
    uint8_t g_final = (g_new * alpha + g_old * (255 - alpha)) / 255;
    uint8_t b_final = (b_new * alpha + b_old * (255 - alpha)) / 255;
    
    uint32_t final_color = (r_final << 16) | (g_final << 8) | b_final;
    buffer[y * fb->width + x] = final_color;
}

void fb_clear(uint32_t color) {
    if (!fb) return;
    
    uint32_t *buffer = fb_get_current_buffer();
    size_t pixel_count = fb->width * fb->height;
    
    for (size_t i = 0; i < pixel_count; i++) {
        buffer[i] = color;
    }
}

void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!fb) return;
    
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            fb_put_pixel(x + j, y + i, color);
        }
    }
}

void fb_draw_rect_alpha(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, uint8_t alpha) {
    if (!fb) return;
    
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            fb_put_pixel_alpha(x + j, y + i, color, alpha);
        }
    }
}

void fb_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color) {
    // Bresenham's line algorithm
    int dx = abs((int)x1 - (int)x0);
    int dy = abs((int)y1 - (int)y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        fb_put_pixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void fb_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color) {
    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        fb_put_pixel(cx + x, cy + y, color);
        fb_put_pixel(cx + y, cy + x, color);
        fb_put_pixel(cx - y, cy + x, color);
        fb_put_pixel(cx - x, cy + y, color);
        fb_put_pixel(cx - x, cy - y, color);
        fb_put_pixel(cx - y, cy - x, color);
        fb_put_pixel(cx + y, cy - x, color);
        fb_put_pixel(cx + x, cy - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg_color, uint32_t bg_color) {
    if (!fb || c < 0 || c > 255) return;
    
    const uint8_t *glyph = font_8x16[(unsigned char)c];
    
    for (int row = 0; row < 16; row++) {
        uint8_t line = glyph[row];
        for (int col = 0; col < 8; col++) {
            uint32_t color = (line & (0x80 >> col)) ? fg_color : bg_color;
            fb_put_pixel(x + col, y + row, color);
        }
    }
}

void fb_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg_color, uint32_t bg_color) {
    if (!str) return;
    
    uint32_t cur_x = x;
    while (*str) {
        if (*str == '\n') {
            cur_x = x;
            y += 16;
        } else {
            fb_draw_char(cur_x, y, *str, fg_color, bg_color);
            cur_x += 8;
        }
        str++;
    }
}

void fb_copy_region(uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y, 
                   uint32_t width, uint32_t height) {
    if (!fb) return;
    
    uint32_t *buffer = fb_get_current_buffer();
    
    // Handle overlapping regions by copying in the right direction
    if (src_y < dst_y || (src_y == dst_y && src_x < dst_x)) {
        // Copy from bottom-right to top-left
        for (int y = height - 1; y >= 0; y--) {
            for (int x = width - 1; x >= 0; x--) {
                uint32_t src_pixel = buffer[(src_y + y) * fb->width + (src_x + x)];
                buffer[(dst_y + y) * fb->width + (dst_x + x)] = src_pixel;
            }
        }
    } else {
        // Copy from top-left to bottom-right
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint32_t src_pixel = buffer[(src_y + y) * fb->width + (src_x + x)];
                buffer[(dst_y + y) * fb->width + (dst_x + x)] = src_pixel;
            }
        }
    }
}

void fb_get_info(struct fb_info *info) {
    if (!info || !fb) return;
    
    info->width = fb->width;
    info->height = fb->height;
    info->bpp = fb->bpp;
    info->pitch = fb->pitch;
    info->address = (uint64_t)fb->address;
    info->double_buffering = double_buffering;
}

// Graphics acceleration functions (software implementation)
void fb_fill_gradient(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                     uint32_t color1, uint32_t color2, bool horizontal) {
    if (!fb) return;
    
    uint8_t r1 = (color1 >> 16) & 0xFF, g1 = (color1 >> 8) & 0xFF, b1 = color1 & 0xFF;
    uint8_t r2 = (color2 >> 16) & 0xFF, g2 = (color2 >> 8) & 0xFF, b2 = color2 & 0xFF;
    
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            uint32_t t_fixed;
            if (horizontal) {
                t_fixed = (j * 256) / (width - 1);  // Fixed point with 8 bits fraction
            } else {
                t_fixed = (i * 256) / (height - 1);
            }
            
            uint8_t r = r1 + ((t_fixed * (r2 - r1)) >> 8);
            uint8_t g = g1 + ((t_fixed * (g2 - g1)) >> 8);
            uint8_t b = b1 + ((t_fixed * (b2 - b1)) >> 8);
            
            uint32_t color = (r << 16) | (g << 8) | b;
            fb_put_pixel(x + j, y + i, color);
        }
    }
}
