#include "mouse.h"
#include "ports.h"
#include "pic.h"

static mouse_state_t mstate = {400, 300, 0};
static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(uint8_t a) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, a);
}

void mouse_init(void) {
    mouse_wait(1);
    outb(0x64, 0xA8); 
    mouse_wait(1);
    outb(0x64, 0x20); 
    mouse_wait(0);
    uint8_t status = (inb(0x60) | 2); 
    mouse_wait(1);
    outb(0x64, 0x60); 
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF4); 
    pic_clear_mask(12);
}

void mouse_irq_handler(void) {
    uint8_t status = inb(0x64);
    if (!(status & 1) || !(status & 0x20)) return;
    
    mouse_byte[mouse_cycle++] = inb(0x60);
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40) return;
        
        mstate.x += (int8_t)mouse_byte[1];
        mstate.y -= (int8_t)mouse_byte[2];
        mstate.buttons = mouse_byte[0] & 0x07;
        
        if (mstate.x < 0) mstate.x = 0;
        if (mstate.y < 0) mstate.y = 0;
        if (mstate.x > 800) mstate.x = 800; 
        if (mstate.y > 600) mstate.y = 600;
    }
}

mouse_state_t* mouse_get_state(void) {
    return &mstate;
}
