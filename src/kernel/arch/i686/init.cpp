#include "debug.hpp"
#include "panic.hpp"
#include "acpi/acpi.hpp"
#include "arch/arch.hpp"
#include "arch/i686/idt.hpp"
#include "arch/i686/io.hpp"
#include "arch/i686/multiboot.hpp"
#include "arch/i686/pic.hpp"
#include "arch/i686/ring3.hpp"
#include "arch/i686/tss.hpp"
#include "driver/pci.hpp"
#include "mm/pmm.hpp"
#include "mm/vmm.hpp"
#include "tty/tty.hpp"

extern "C" {
    extern uint32_t kernel_pageDirectory[1024];
    extern kernel::multiboot_info_t kernel_multibootInfo;
    extern kernel::multiboot_info_mmap_entry_t kernel_memoryMap[];
    extern uint32_t kernel_memoryMapLength;
    extern uint32_t kernel_multibootMagic;
}

namespace kernel {
    TTY kernel_tty = TTY((void *)0xc00b8000, 80, 25);

    void arch_preinit() {
        kernel_tty.cls();
        
        if(kernel_multibootMagic == 0x2badb002) {
            debug("Good multiboot signature.\n");
        } else {
            panic("Wrong multiboot signature.");
        }

        if(!(kernel_multibootInfo.flags & (1 << 6))) {
            kernel_tty.setAttr(0x0c);
            debug("The bootloader did not provide a memory map. System halted.\n");
            halt();
        }

        debug("Initializing PMM... ");
        pmm_init(kernel_memoryMap, kernel_memoryMapLength / sizeof(multiboot_info_mmap_entry_t));
        debug("Done.\n");
    }

    void arch_init() {
        debug("Initializing IDT... ");
        idt_init();
        debug("Done.\n");

        debug("Initializing PIC... ");
        pic_init();
        debug("Done.\n");

        debug("Enabling interrupts... ");
        sti();
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
    }
}