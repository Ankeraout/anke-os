#include <stdbool.h>
#include <stdint.h>

#include "arch/i686/multiboot.h"
#include "libk/libk.h"
#include "tty/tty.h"

tty_t kernel_tty = {
    .x = 0,
    .y = 0,
    .w = 80,
    .h = 25,
    .buf = (void *)0xc00b8000,
    .attr = 0x07
};

void kernel_main(uint32_t multiboot_magic, const multiboot_info_t *multiboot_info) {
    UNUSED_PARAMETER(multiboot_info);

    tty_cls(&kernel_tty);
    kernel_tty.attr = 0x0a;
    
    tty_puts(&kernel_tty, "       db                      88                      ,ad8888ba,     ad88888ba ");
    tty_puts(&kernel_tty, "      d88b                     88                     d8\"'    `\"8b   d8\"     \"8b");
    tty_puts(&kernel_tty, "     d8'`8b                    88                    d8'        `8b  Y8,        ");
    tty_puts(&kernel_tty, "    d8'  `8b      8b,dPPYba,   88   ,d8   ,adPPYba,  88          88  `Y8aaaaa,  ");
    tty_puts(&kernel_tty, "   d8YaaaaY8b     88P'   `\"8a  88 ,a8\"   a8P_____88  88          88    `\"\"\"\"\"8b,");
    tty_puts(&kernel_tty, "  d8\"\"\"\"\"\"\"\"8b    88       88  8888[     8PP\"\"\"\"\"\"\"  Y8,        ,8P          `8b");
    tty_puts(&kernel_tty, " d8'        `8b   88       88  88`\"Yba,  \"8b,   ,aa   Y8a.    .a8P   Y8a     a8P");
    tty_puts(&kernel_tty, "d8'          `8b  88       88  88   `Y8a  `\"Ybbd8\"'    `\"Y8888Y\"'     \"Y88888P\" ");
    tty_putc(&kernel_tty, '\n');
    kernel_tty.attr = 0x07;
    tty_puts(&kernel_tty, "Welcome to AnkeOS!\n");

    if(multiboot_magic == 0x2badb002) {
        tty_puts(&kernel_tty, "Good multiboot signature.\n");
    } else {
        tty_puts(&kernel_tty, "Wrong multiboot signature.\n");
    }

    while(true) {
        asm("cli\n \
             hlt");
    }
}
