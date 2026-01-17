#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H

#include <stdarg.h>

void printk(const char *fmt, ...);
int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);

#endif
