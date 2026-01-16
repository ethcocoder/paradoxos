#include "keyboard.h"
#include "ports.h"
#include "pic.h"

static char last_key = 0;

static const char scancode_map[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void keyboard_init(void) {
    pic_clear_mask(1);
    while (inb(0x64) & 1) inb(0x60);
}

void keyboard_irq_handler(void) {
    uint8_t scancode = inb(0x60);
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(scancode_map)) {
            last_key = scancode_map[scancode];
        }
    }
}

char keyboard_get_last_key(void) {
    char k = last_key;
    last_key = 0;
    return k;
}
