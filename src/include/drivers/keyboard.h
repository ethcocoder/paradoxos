#ifndef _DRIVERS_KEYBOARD_H
#define _DRIVERS_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
char keyboard_get_key(void); // Non-blocking check
void keyboard_handler_c(void); // IRQ handler

// Mouse functions
void mouse_handler_c(void); // IRQ12 handler
void get_mouse_state(int *x, int *y, bool *left, bool *right, bool *middle);

// Input state
bool is_shift_pressed(void);
bool is_ctrl_pressed(void);
bool is_alt_pressed(void);

#endif
