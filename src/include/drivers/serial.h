#ifndef _DRIVERS_SERIAL_H
#define _DRIVERS_SERIAL_H

#include <stdint.h>

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char* str);

#endif
