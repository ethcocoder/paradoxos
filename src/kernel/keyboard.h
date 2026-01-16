#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
char keyboard_get_last_key(void);
void keyboard_irq_handler(void);

#endif
