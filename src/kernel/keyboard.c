#include "keyboard.h"
#include "ports.h"
#include "cpu.h"
#include "gfx.h"
#include "font.h"

extern struct limine_framebuffer_request framebuffer_request;

/* Scancode map for US QWERTY */
static const char scancode_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static char last_key = 0;

char keyboard_get_last_key() {
    char c = last_key;
    last_key = 0; // Consume the key
    return c;
}

__attribute__((interrupt))
void keyboard_handler(void* frame) {
    uint8_t scancode = inb(0x60);
    
    // Only handle key press (ignore release)
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(scancode_map)) {
            last_key = scancode_map[scancode];
            
        }
    }

    // Acknowledge PIC
    outb(0x20, 0x20);
    (void)frame;
}

void keyboard_init() {
    /* 1. Remap PIC */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master offset 32
    outb(0xA1, 0x28); // Slave offset 40
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    /* 2. Register Keyboard Interrupt (33 since Master PIT is 32) */
    idt_set_descriptor(33, keyboard_handler, 0x8E);
}
