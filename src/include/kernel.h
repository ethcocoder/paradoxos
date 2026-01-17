#ifndef _KERNEL_H
#define _KERNEL_H

#include <paradox.h>

// Global Kernel Functions
void kernel_startup(void);
void kernel_panic(const char *msg);
void display_system_status(void);

#endif
