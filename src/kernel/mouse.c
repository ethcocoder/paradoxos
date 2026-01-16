#include "mouse.h"
#include "ports.h"
#include "cpu.h"
#include "gfx.h"
#include "../boot/limine.h"

extern struct limine_framebuffer_request framebuffer_request;

static mouse_state_t m_state = {0, 0, 0, 0, 0};
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    while (timeout--) {
        if (type == 0) {
            if ((inb(0x64) & 1) == 1) return;
        } else {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

static uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

__attribute__((interrupt))
void mouse_handler(void* frame) {
    uint8_t status = inb(0x64);
    if (!(status & 1) || !(status & 0x20)) {
        goto end;
    }

    mouse_byte[mouse_cycle++] = inb(0x60);

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40) goto end;

        m_state.left_button = mouse_byte[0] & 1;
        m_state.right_button = (mouse_byte[0] & 2) >> 1;
        m_state.middle_button = (mouse_byte[0] & 4) >> 2;

        int x_move = mouse_byte[1];
        int y_move = mouse_byte[2];

        if (mouse_byte[0] & 0x10) x_move -= 256;
        if (mouse_byte[0] & 0x20) y_move -= 256;

        m_state.x += x_move;
        m_state.y -= y_move; // Y is inverted in PS/2 vs Screen coords

        // Clamp to screen bounds
        if (framebuffer_request.response) {
            uint32_t w = framebuffer_request.response->framebuffers[0]->width;
            uint32_t h = framebuffer_request.response->framebuffers[0]->height;
            if (m_state.x < 0) m_state.x = 0;
            if (m_state.y < 0) m_state.y = 0;
            if (m_state.x > (int)w - 1) m_state.x = w - 1;
            if (m_state.y > (int)h - 1) m_state.y = h - 1;
        }
    }

end:
    // Acknowledge both PICs
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
    (void)frame;
}

void mouse_init() {
    uint8_t status;

    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // Tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();

    // Enable data reporting
    mouse_write(0xF4);
    mouse_read();

    // Setup IDT entry (IRQ 12 is interrupt 44)
    idt_set_descriptor(44, mouse_handler, 0x8E);
    
    // Unmask IRQ 12 on Slave PIC
    outb(0xA1, inb(0xA1) & ~(1 << 4));
}

mouse_state_t* mouse_get_state() {
    return &m_state;
}
