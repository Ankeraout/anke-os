#include <stdint.h>

#include "asm.h"

static unsigned int s_consoleX = 0;
static unsigned int s_consoleY = 0;
static uint8_t *s_vga = (uint8_t *)0xb8000;

static void update_cursor(void);

void stdio_init(void) {
    outb(0x3d4, 0x0e);
    unsigned int l_cursorIndex = inb(0x3d5) << 8;
    outb(0x3d4, 0x0f);
    l_cursorIndex |= inb(0x3d5);

    s_consoleY = l_cursorIndex / 80;
    s_consoleX = l_cursorIndex % 80;
}

void putc(int p_c) {
    if(p_c == 0) {
        return;
    } else if(p_c == '\r') {
        s_consoleX = 0;
    } else if(p_c == '\n') {
        s_consoleX = 0;
        s_consoleY++;
    } else {
        unsigned int l_index = (s_consoleY * 80 + s_consoleX) << 1;
        s_vga[l_index] = p_c;
        s_vga[l_index + 1] = 0x07;
        s_consoleX++;
    }

    if(s_consoleX == 80) {
        s_consoleY++;
        s_consoleX = 0;
    }

    if(s_consoleY == 25) {
        s_consoleY = 0;
    }

    update_cursor();
}

void puts(const char *p_str) {
    while(*p_str != 0) {
        putc(*p_str++);
    }
}

void update_cursor(void) {
    unsigned int l_cursorIndex = s_consoleY * 80 + s_consoleX;

    outb(0x3d4, 0x0e);
    outb(0x3d5, l_cursorIndex >> 8);
    outb(0x3d4, 0x0f);
    outb(0x3d5, l_cursorIndex);
}
