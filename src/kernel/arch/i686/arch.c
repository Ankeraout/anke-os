#include "arch/i686/bioscall.h"
#include "arch/i686/idt.h"
#include "arch/i686/io.h"
#include "arch/i686/mmap.h"
#include "arch/i686/pci.h"
#include "arch/i686/pic.h"
#include "arch/i686/pit.h"
#include "arch/i686/ring3.h"
#include "arch/i686/tss.h"
#include "acpi.h"
#include "arch/i686/mm/mm.h"
#include "arch/i686/mm/pmm.h"
#include "arch/i686/mm/vmm.h"
#include "arch/i686/tty/bios.h"
#include "arch/i686/tty/text16.h"

#include "libk/stdio.h"

void arch_preinit();
void arch_init();
static void arch_setCursorPosition(tty_t *tty, int x, int y);
void arch_disableInterrupts();
void arch_halt();

void arch_preinit() {
    bioscall_init();

    bioscall_context_t bioscall_context = {
        .ax = 0x1112,
        .bl = 0x00
    };

    bioscall(0x10, &bioscall_context);

    tty_bios_init(&kernel_tty);

    printf("arch: initializing IDT...\n");
    idt_init();
    printf("arch: initializing PIC...\n");
    pic_init();
    printf("arch: initializing PIT...\n");
    pit_init();

    asm("sti");
    printf("arch: interrupts are now enabled\n");

    printf("arch: initializing memory manager...\n");
    mm_init();
    printf("arch: initializing memory map...\n");
    mmap_init();
    printf("arch: initializing physical memory manager\n");
    pmm_init();
    printf("arch: initializing virtual memory manager\n");
    vmm_init();
    printf("arch: initializing tty driver...\n");

    void *tty_buffer = vmm_map((const void *)0xb8000, 2, true);

    int textCursorX;
    int textCursorY;

    kernel_tty.api.getCursorPosition(&kernel_tty, &textCursorX, &textCursorY);

    tty_text16_init(&kernel_tty, tty_buffer, 80, 50, arch_setCursorPosition);

    kernel_tty.api.setCursorPosition(&kernel_tty, textCursorX, textCursorY);
}

void arch_init() {
    pci_init();
    acpi_init();

    tss_init();
    tss_load();
}

static void arch_setCursorPosition(tty_t *tty, int x, int y) {
    uint16_t cursorPosition = y * tty_getTerminalWidth(tty) + x;

    outb(0x3d4, 0x0f);
    outb(0x3d5, cursorPosition);
    outb(0x3d4, 0x0e);
    outb(0x3d5, cursorPosition >> 8);
}

void arch_disableInterrupts() {
    asm("cli");
}

void arch_halt() {
    asm("hlt");
}
