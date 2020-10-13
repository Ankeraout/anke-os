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

void halt() {
    while(true) {
        asm("cli\n \
            hlt");
    }
}

void kernel_main(uint32_t multiboot_magic, const multiboot_info_t *multiboot_info) {
    UNUSED_PARAMETER(multiboot_info);

    tty_cls(&kernel_tty);
    kernel_tty.attr = 0x0f;
    
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

    if(multiboot_magic == 0x2badb002) {
        kernel_tty.attr = 0x0a;
        tty_puts(&kernel_tty, "Good multiboot signature.\n");
        kernel_tty.attr = 0x07;
    } else {
        kernel_tty.attr = 0x0c;
        tty_puts(&kernel_tty, "Wrong multiboot signature. System halted.\n");
        halt();
    }

    tty_puts(&kernel_tty, "Welcome to AnkeOS!");

    if(multiboot_info->flags & (1 << 9)) {
        tty_puts(&kernel_tty, " (booted using ");
        tty_puts(&kernel_tty, (char *)multiboot_info->boot_loader_name);
        tty_puts(&kernel_tty, ")");
    }

    tty_putc(&kernel_tty, '\n');

    if(!(multiboot_info->flags & (1 << 6))) {
        kernel_tty.attr = 0x0c;
        tty_puts(&kernel_tty, "The bootloader did not provide a memory map. System halted.\n");
        halt();
    }

    tty_puts(&kernel_tty, "\nMemory map:\n");

    multiboot_info_mmap_entry_t *memoryMapEntry = (multiboot_info_mmap_entry_t *)multiboot_info->mmap_addr;
    size_t size = multiboot_info->mmap_length / sizeof(multiboot_info_mmap_entry_t);
    size_t free = 0;
    char buffer[33];

    for(size_t i = 0; i < size; i++) {
        tty_puts(&kernel_tty, "addr=0x");
        tty_puts(&kernel_tty, hex64(memoryMapEntry[i].base_addr, buffer));
        tty_puts(&kernel_tty, " len=0x");
        tty_puts(&kernel_tty, hex64(memoryMapEntry[i].length, buffer));
        tty_puts(&kernel_tty, " type=");
        tty_puts(&kernel_tty, itoa(memoryMapEntry[i].type, buffer, 10));
        tty_putc(&kernel_tty, '\n');

        if(memoryMapEntry[i].type == 1) {
            free += memoryMapEntry[i].length;
        }
    }

    tty_puts(&kernel_tty, "\nTotal usable memory: ");
    tty_puts(&kernel_tty, itoa(free >> 10, buffer, 10));
    tty_puts(&kernel_tty, " KiB\n");

    halt();
}
