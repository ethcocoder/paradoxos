#include <stddef.h>
#include <stdint.h>

#include "boot/stivale2.h"
#include "gfx.h"
#include "serial.h"
#include "logo.h"
#include "memory/pmm.h"
#include "memory/slab.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "mouse.h"
#include "font.h"

void kmain(struct stivale2_struct *stivale2_struct) {
    framebuffer_init(stivale2_struct, 0x000000); 
    serial_init();

    serial_send_string("[INFO] Framebuffer and serial initialized\n");
    serial_send_string("[INFO] Welcome to ParadoxOS\n");
    serial_send_string(paradox_logo_text);

    font_draw_string("Paradox Engine v2.5", 20, 20, 0xFF00AAFF);
    font_draw_string("Initializing Paradox Core...", 20, 40, 0xFFFFFFFF);

    pmm_init(stivale2_struct);
    font_draw_string("⤷ PMM: READY", 20, 60, 0xFF00FF00);
    
    gdt_init();
    font_draw_string("⤷ GDT: OK", 20, 80, 0xFF00FF00);
    
    idt_init();
    font_draw_string("⤷ IDT: OK", 20, 100, 0xFF00FF00);
    
    slab_init();
    font_draw_string("⤷ Slab: READY", 20, 120, 0xFF00FF00);

    keyboard_init();
    mouse_init();
    font_draw_string("⤷ IRQ Drivers: OK", 20, 140, 0xFF00FF00);

    font_draw_string("ParadoxOS is now active.", 20, 180, 0xFFFFAA00);

    for (;;) {
        __asm__("hlt");
    }
}
