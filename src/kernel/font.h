#ifndef FONT_H
#define FONT_H

#include <stdint.h>

/* Simple 8x16 font data (Assimilated) */
extern uint8_t font_bitmap[128][16];

void font_draw_char(char c, int x, int y, uint32_t color);
void font_draw_string(const char *str, int x, int y, uint32_t color);

#endif
