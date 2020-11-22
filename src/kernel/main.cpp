#include <stdbool.h>
#include <stdint.h>

#include "debug.hpp"
#include "acpi/acpi.hpp"
#include "arch/i686/idt.hpp"
#include "arch/i686/io.hpp"
#include "arch/i686/multiboot.hpp"
#include "arch/i686/pic.hpp"
#include "arch/i686/ring3.hpp"
#include "arch/i686/tss.hpp"
#include "driver/pci.hpp"
#include "libk/libk.hpp"
#include "mm/pmm.hpp"
#include "mm/vmm.hpp"
#include "tty/tty.hpp"

extern "C" {
    extern uint32_t kernel_pageDirectory[1024];
    extern kernel::multiboot_info_t kernel_multibootInfo;
    extern kernel::multiboot_info_mmap_entry_t kernel_memoryMap[];
    extern uint32_t kernel_memoryMapLength;
}

namespace kernel {
    TTY kernel_tty = TTY((void *)0xc00b8000, 80, 25);

    void halt() {
        while(true) {
            asm("cli\n \
                hlt");
        }
    }

    void kmain(uint32_t multiboot_magic) {
        kernel_tty.cls();
        kernel_tty.setAttr(0x0f);
        debug("       db                      88                      ,ad8888ba,     ad88888ba ");
        debug("      d88b                     88                     d8\"'    `\"8b   d8\"     \"8b");
        debug("     d8'`8b                    88                    d8'        `8b  Y8,        ");
        debug("    d8'  `8b      8b,dPPYba,   88   ,d8   ,adPPYba,  88          88  `Y8aaaaa,  ");
        debug("   d8YaaaaY8b     88P'   `\"8a  88 ,a8\"   a8P_____88  88          88    `\"\"\"\"\"8b,");
        debug("  d8\"\"\"\"\"\"\"\"8b    88       88  8888[     8PP\"\"\"\"\"\"\"  Y8,        ,8P          `8b");
        debug(" d8'        `8b   88       88  88`\"Yba,  \"8b,   ,aa   Y8a.    .a8P   Y8a     a8P");
        debug("d8'          `8b  88       88  88   `Y8a  `\"Ybbd8\"'    `\"Y8888Y\"'     \"Y88888P\" ");
        kernel_tty.putc('\n');
        kernel_tty.setAttr(0x07);

        if(multiboot_magic == 0x2badb002) {
            kernel_tty.setAttr(0x0a);
            debug("Good multiboot signature.\n");
            kernel_tty.setAttr(0x07);
        } else {
            kernel_tty.setAttr(0x0c);
            debug("Wrong multiboot signature. System halted.\n");
            halt();
        }

        debug("Welcome to AnkeOS!");

        kernel_tty.putc('\n');

        if(!(kernel_multibootInfo.flags & (1 << 6))) {
            kernel_tty.setAttr(0x0c);
            debug("The bootloader did not provide a memory map. System halted.\n");
            halt();
        }
        
        debug("Initializing IDT... ");
        idt_init();
        debug("Done.\n");

        debug("Initializing PIC... ");
        pic_init();
        debug("Done.\n");

        debug("Enabling interrupts... ");
        sti();
        debug("Done.\n");

        debug("Initializing PMM... ");
        pmm_init(kernel_memoryMap, kernel_memoryMapLength / sizeof(multiboot_info_mmap_entry_t));
        debug("Done.\n");

        debug("Initializing ACPI...\n");
        acpi_init();
        debug("Done.\n");

        debug("Initializing PCI... ");
        pci_init(PCI_CSAM_2);
        debug("Done.\n");

        debug("Initializing TSS... ");
        tss_init();
        tss_flush();
        debug("Done.\n");

        while(true) {
            hlt();
        }
    }
}

extern "C" void kernel_main(uint32_t multiboot_magic) {
    kernel::kmain(multiboot_magic);
}
