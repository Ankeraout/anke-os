#include <stdbool.h>
#include <stdint.h>

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
        
        kernel_tty.puts("       db                      88                      ,ad8888ba,     ad88888ba ");
        kernel_tty.puts("      d88b                     88                     d8\"'    `\"8b   d8\"     \"8b");
        kernel_tty.puts("     d8'`8b                    88                    d8'        `8b  Y8,        ");
        kernel_tty.puts("    d8'  `8b      8b,dPPYba,   88   ,d8   ,adPPYba,  88          88  `Y8aaaaa,  ");
        kernel_tty.puts("   d8YaaaaY8b     88P'   `\"8a  88 ,a8\"   a8P_____88  88          88    `\"\"\"\"\"8b,");
        kernel_tty.puts("  d8\"\"\"\"\"\"\"\"8b    88       88  8888[     8PP\"\"\"\"\"\"\"  Y8,        ,8P          `8b");
        kernel_tty.puts(" d8'        `8b   88       88  88`\"Yba,  \"8b,   ,aa   Y8a.    .a8P   Y8a     a8P");
        kernel_tty.puts("d8'          `8b  88       88  88   `Y8a  `\"Ybbd8\"'    `\"Y8888Y\"'     \"Y88888P\" ");
        kernel_tty.putc('\n');
        kernel_tty.setAttr(0x07);

        if(multiboot_magic == 0x2badb002) {
            kernel_tty.setAttr(0x0a);
            kernel_tty.puts("Good multiboot signature.\n");
            kernel_tty.setAttr(0x07);
        } else {
            kernel_tty.setAttr(0x0c);
            kernel_tty.puts("Wrong multiboot signature. System halted.\n");
            halt();
        }

        kernel_tty.puts("Welcome to AnkeOS!");

        kernel_tty.putc('\n');

        if(!(kernel_multibootInfo.flags & (1 << 6))) {
            kernel_tty.setAttr(0x0c);
            kernel_tty.puts("The bootloader did not provide a memory map. System halted.\n");
            halt();
        }
        
        kernel_tty.puts("Initializing IDT... ");
        idt_init();
        kernel_tty.puts("Done.\n");

        kernel_tty.puts("Initializing PIC... ");
        pic_init();
        kernel_tty.puts("Done.\n");

        kernel_tty.puts("Enabling interrupts... ");
        sti();
        kernel_tty.puts("Done.\n");

        kernel_tty.puts("Initializing PMM... ");
        pmm_init(kernel_memoryMap, kernel_memoryMapLength / sizeof(multiboot_info_mmap_entry_t));
        kernel_tty.puts("Done.\n");

        kernel_tty.puts("Initializing ACPI...\n");
        acpi_init();
        kernel_tty.puts("Done.\n");

        kernel_tty.puts("Initializing PCI... ");
        pci_init(PCI_CSAM_2);
        kernel_tty.puts("Done.\n");

        kernel_tty.puts("Initializing TSS... ");
        tss_init();
        tss_flush();
        kernel_tty.puts("Done.\n");

        while(true) {
            hlt();
        }
    }
}

extern "C" void kernel_main(uint32_t multiboot_magic) {
    kernel::kmain(multiboot_magic);
}
