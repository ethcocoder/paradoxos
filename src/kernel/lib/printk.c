#include <lib/stdio.h>
#include <drivers/serial.h>
#include <drivers/framebuffer.h>
#include <lib/string.h>
#include <stdint.h>
#include <stddef.h>

static void print_char(char c) {
    serial_putc(c);
    // TODO: Connect to Framebuffer console here when font renderer is ready
}

static void print_str(const char *s) {
    while (*s) {
        print_char(*s++);
    }
}

static void print_dec(long long value) {
    char buffer[21];
    int i = 0;
    int sign = 0;

    if (value < 0) {
        sign = 1;
        value = -value;
    }

    if (value == 0) {
        print_char('0');
        return;
    }

    while (value > 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (sign) {
        print_char('-');
    }

    while (i > 0) {
        print_char(buffer[--i]);
    }
}

static void print_hex(unsigned long long value) {
    char buffer[17];
    int i = 0;
    const char *hex_chars = "0123456789ABCDEF";

    if (value == 0) {
        print_str("0x0");
        return;
    }

    while (value > 0) {
        buffer[i++] = hex_chars[value % 16];
        value /= 16;
    }

    print_str("0x");
    while (i > 0) {
        print_char(buffer[--i]);
    }
}

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'c': {
                    int c = va_arg(args, int);
                    print_char(c);
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, const char *);
                    print_str(s ? s : "(null)");
                    break;
                }
                case 'd': {
                    long long d = va_arg(args, long long);
                    print_dec(d);
                    break;
                }
                case 'x':
                case 'p': {
                    unsigned long long x = va_arg(args, unsigned long long);
                    print_hex(x);
                    break;
                }
                case '%': {
                    print_char('%');
                    break;
                }
                default:
                    print_char('%');
                    print_char(*fmt);
                    break;
            }
        } else {
            print_char(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
