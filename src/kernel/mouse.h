#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int x, y;
    int buttons;
} mouse_state_t;

void mouse_init(void);
void mouse_irq_handler(void);
mouse_state_t* mouse_get_state(void);

#endif
