#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include "gfx.h"

void font_draw_char(char c, uint32_t x, uint32_t y, color_t color);
void font_draw_string(const char *str, uint32_t x, uint32_t y, color_t color);

#endif
