#include "serial.h"
#include "ports.h"

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_send_char(char c) {
    while (is_transmit_empty() == 0);
    outb(COM1, c);
}

void serial_send_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++)
        serial_send_char(str[i]);
}
