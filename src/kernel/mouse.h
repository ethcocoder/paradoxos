#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

void mouse_init();
void mouse_handler();

typedef struct {
    int x;
    int y;
    uint8_t left_button;
    uint8_t right_button;
    uint8_t middle_button;
} mouse_state_t;

mouse_state_t* mouse_get_state();

#endif
