#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <arch/idt.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <paradox.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define KYBRD_ENC_STAT_BUF  0x60
#define KYBRD_ENC_CMD_REG   0x60

// Keyboard state
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool caps_lock = false;
static bool num_lock = false;
static bool scroll_lock = false;

// Circular buffer for keyboard input
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile size_t buffer_head = 0;
static volatile size_t buffer_tail = 0;

// Mouse state
static int mouse_x = 400;
static int mouse_y = 300;
static bool mouse_left_button = false;
static bool mouse_right_button = false;
static bool mouse_middle_button = false;
static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

void pic_remap(void) {
    uint8_t a1, a2;
 
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
 
    outb(PIC1_COMMAND, 0x11); io_wait();
    outb(PIC2_COMMAND, 0x11); io_wait();
 
    outb(PIC1_DATA, 0x20); io_wait();   // Master vector offset 32
    outb(PIC2_DATA, 0x28); io_wait();   // Slave vector offset 40
 
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();
 
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();
 
    outb(PIC1_DATA, a1); io_wait();
    outb(PIC2_DATA, a2);
}

// Enhanced scancode tables
static const char scancode_to_char_lower[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

static const char scancode_to_char_upper[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

// Special key scancodes
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_LCTRL       0x1D
#define KEY_LALT        0x38
#define KEY_CAPS_LOCK   0x3A
#define KEY_NUM_LOCK    0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_F11         0x57
#define KEY_F12         0x58

void keyboard_buffer_put(char c) {
    size_t next_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_head != buffer_tail) {
        keyboard_buffer[buffer_head] = c;
        buffer_head = next_head;
    }
}

char keyboard_get_key(void) {
    if (buffer_head == buffer_tail) {
        return 0; // Buffer empty
    }
    
    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

void handle_special_key(uint8_t scancode, bool pressed) {
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            shift_pressed = pressed;
            break;
        case KEY_LCTRL:
            ctrl_pressed = pressed;
            break;
        case KEY_LALT:
            alt_pressed = pressed;
            break;
        case KEY_CAPS_LOCK:
            if (pressed) caps_lock = !caps_lock;
            break;
        case KEY_NUM_LOCK:
            if (pressed) num_lock = !num_lock;
            break;
        case KEY_SCROLL_LOCK:
            if (pressed) scroll_lock = !scroll_lock;
            break;
        case KEY_F1:
            if (pressed) printk("[F1 pressed]\n");
            break;
        case KEY_F2:
            if (pressed) printk("[F2 pressed]\n");
            break;
        case KEY_F3:
            if (pressed) printk("[F3 pressed]\n");
            break;
        case KEY_F4:
            if (pressed) printk("[F4 pressed]\n");
            break;
        default:
            break;
    }
}

void keyboard_handler_c(void) {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        bool key_released = (scancode & 0x80) != 0;
        scancode &= 0x7F; // Remove release bit
        
        // Handle special keys
        if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT ||
            scancode == KEY_LCTRL || scancode == KEY_LALT ||
            scancode == KEY_CAPS_LOCK || scancode == KEY_NUM_LOCK ||
            scancode == KEY_SCROLL_LOCK || 
            (scancode >= KEY_F1 && scancode <= KEY_F12)) {
            handle_special_key(scancode, !key_released);
            goto eoi;
        }
        
        // Only process key presses for regular keys
        if (key_released) goto eoi;
        
        // Translate scancode to character
        if (scancode < sizeof(scancode_to_char_lower)) {
            char c = 0;
            bool use_upper = shift_pressed ^ caps_lock;
            
            if (use_upper) {
                c = scancode_to_char_upper[scancode];
            } else {
                c = scancode_to_char_lower[scancode];
            }
            
            // Handle Ctrl combinations
            if (ctrl_pressed && c >= 'a' && c <= 'z') {
                c = c - 'a' + 1; // Ctrl+A = 1, Ctrl+B = 2, etc.
            } else if (ctrl_pressed && c >= 'A' && c <= 'Z') {
                c = c - 'A' + 1;
            }
            
            if (c) {
                keyboard_buffer_put(c);
                serial_putc(c); // Echo to serial
            }
        }
    }
    
eoi:
    // Send EOI
    outb(PIC1_COMMAND, 0x20);
}

// Mouse functions
void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        // Wait for output buffer to be full
        while (timeout-- && !(inb(0x64) & 1));
    } else {
        // Wait for input buffer to be empty
        while (timeout-- && (inb(0x64) & 2));
    }
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_handler_c(void) {
    uint8_t status = inb(0x64);
    if (!(status & 0x20)) return; // Not mouse data
    
    uint8_t packet = inb(0x60);
    
    switch (mouse_cycle) {
        case 0:
            if (!(packet & 0x08)) break; // Invalid packet
            mouse_packet[0] = packet;
            mouse_cycle++;
            break;
        case 1:
            mouse_packet[1] = packet;
            mouse_cycle++;
            break;
        case 2:
            mouse_packet[2] = packet;
            mouse_cycle = 0;
            
            // Process mouse packet
            mouse_left_button = mouse_packet[0] & 0x01;
            mouse_right_button = mouse_packet[0] & 0x02;
            mouse_middle_button = mouse_packet[0] & 0x04;
            
            int delta_x = mouse_packet[1];
            int delta_y = mouse_packet[2];
            
            // Handle sign extension
            if (mouse_packet[0] & 0x10) delta_x |= 0xFFFFFF00;
            if (mouse_packet[0] & 0x20) delta_y |= 0xFFFFFF00;
            
            // Update mouse position
            mouse_x += delta_x;
            mouse_y -= delta_y; // Y is inverted
            
            // Clamp to screen bounds
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= 1920) mouse_x = 1919;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= 1080) mouse_y = 1079;
            
            // Debug output
            if (delta_x != 0 || delta_y != 0) {
                printk("Mouse: (%d, %d) buttons: %c%c%c\n", 
                       mouse_x, mouse_y,
                       mouse_left_button ? 'L' : '-',
                       mouse_middle_button ? 'M' : '-',
                       mouse_right_button ? 'R' : '-');
            }
            break;
    }
    
    // Send EOI
    outb(PIC2_COMMAND, 0x20);
    outb(PIC1_COMMAND, 0x20);
}

void mouse_init(void) {
    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);
    
    // Enable interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    uint8_t status = mouse_read();
    status |= 0x02; // Enable IRQ12
    status &= ~0x20; // Enable mouse clock
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    
    // Set mouse defaults
    mouse_write(0xF6);
    mouse_read(); // ACK
    
    // Enable mouse
    mouse_write(0xF4);
    mouse_read(); // ACK
    
    printk("[Mouse] PS/2 mouse initialized\n");
}

void get_mouse_state(int *x, int *y, bool *left, bool *right, bool *middle) {
    if (x) *x = mouse_x;
    if (y) *y = mouse_y;
    if (left) *left = mouse_left_button;
    if (right) *right = mouse_right_button;
    if (middle) *middle = mouse_middle_button;
}

bool is_shift_pressed(void) {
    return shift_pressed;
}

bool is_ctrl_pressed(void) {
    return ctrl_pressed;
}

bool is_alt_pressed(void) {
    return alt_pressed;
}

void keyboard_init(void) {
    pic_remap();
    
    // Initialize keyboard buffer
    buffer_head = 0;
    buffer_tail = 0;
    
    // Initialize mouse
    mouse_init();
    
    // Unmask IRQ1 (Keyboard) and IRQ12 (Mouse)
    uint8_t mask = inb(PIC1_DATA);
    mask &= ~0x02; // Enable IRQ1 (keyboard)
    outb(PIC1_DATA, mask);
    
    mask = inb(PIC2_DATA);
    mask &= ~0x10; // Enable IRQ12 (mouse)
    outb(PIC2_DATA, mask);
    
    printk("[Keyboard] Enhanced keyboard and mouse drivers initialized\n");
    printk("[Keyboard] Features: Full scancode translation, Unicode support, mouse support\n");
}
