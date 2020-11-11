#include <stdbool.h>
#include <stdint.h>

#include "acpi/acpi.h"
#include "arch/i686/idt.h"
#include "arch/i686/io.h"
#include "arch/i686/multiboot.h"
#include "arch/i686/pic.h"
#include "arch/i686/ring3.h"
#include "arch/i686/tss.h"
#include "libk/libk.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "tty/tty.h"

extern uint32_t kernel_pageDirectory[1024];
extern multiboot_info_t kernel_multibootInfo;
extern multiboot_info_mmap_entry_t kernel_memoryMap[];
extern uint32_t kernel_memoryMapLength;

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

void usermodeFunc() {
    tty_puts(&kernel_tty, "This was printed from ring 3.\n");

    asm("int $0x80");

    tty_puts(&kernel_tty, "This was also printed from ring 3.\n");

    while(true);
}

void kernel_main(uint32_t multiboot_magic) {
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

    tty_putc(&kernel_tty, '\n');

    if(!(kernel_multibootInfo.flags & (1 << 6))) {
        kernel_tty.attr = 0x0c;
        tty_puts(&kernel_tty, "The bootloader did not provide a memory map. System halted.\n");
        halt();
    }
    
    tty_puts(&kernel_tty, "Initializing IDT... ");
    idt_init();
    tty_puts(&kernel_tty, "Done.\n");

    tty_puts(&kernel_tty, "Initializing PIC... ");
    pic_init();
    tty_puts(&kernel_tty, "Done.\n");

    tty_puts(&kernel_tty, "Enabling interrupts... ");
    //sti();
    tty_puts(&kernel_tty, "Done.\n");

    tty_puts(&kernel_tty, "Initializing PMM... ");
    pmm_init(kernel_memoryMap, kernel_memoryMapLength / sizeof(multiboot_info_mmap_entry_t));
    tty_puts(&kernel_tty, "Done.\n");

    tty_puts(&kernel_tty, "Initializing ACPI...\n");
    acpi_init();
    tty_puts(&kernel_tty, "Done.\n");

    tty_puts(&kernel_tty, "Initializing TSS... ");
    tss_init();
    tss_flush();
    tty_puts(&kernel_tty, "Done.\n");

    callUsermode(usermodeFunc);

    while(true) {
        hlt();
    }
}
