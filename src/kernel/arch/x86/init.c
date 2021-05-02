#include "kernel/arch/x86/bios.h"
#include "kernel/arch/x86/idt.h"
#include "kernel/arch/x86/mmap.h"
#include "kernel/arch/x86/pic.h"
#include "kernel/arch/x86/mm/mm.h"
#include "kernel/arch/x86/mm/pmm.h"
#include "kernel/arch/x86/mm/vmm.h"

#include "kernel/arch/x86/dev/pci.h"
#include "kernel/arch/x86/dev/keyboard/ps2kbd.h"
#include "kernel/arch/x86/dev/tty/vgaconsole.h"

#include "kernel/libk/stdio.h"

void arch_preinit();
void arch_init();

void arch_preinit() {
    mm_init();
    bios_init();
    mmap_init();
    pmm_init();
    vmm_init();

    vgaconsole_init();
}

void arch_init() {
    idt_init();
    pic_init();

    asm("sti");

    printf("Initializing PCI...\n");
    pci_init();
    ps2kbd_init();
}
