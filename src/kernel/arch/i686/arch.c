#include "arch/i686/bioscall.h"
#include "arch/i686/idt.h"
#include "arch/i686/mmap.h"
#include "arch/i686/pic.h"
#include "arch/i686/video.h"
#include "arch/i686/mm/mm.h"
#include "arch/i686/mm/pmm.h"
#include "arch/i686/mm/vmm.h"
#include "arch/i686/tty/text16.h"

#include "libk/stdio.h"

void arch_init();
static void arch_setCursorPosition(tty_t *tty, int x, int y);
void arch_disableInterrupts();
void arch_halt();

tty_t kernel_tty;

void arch_init() {
    bioscall_init();
    
    idt_init();
    pic_init();

    asm("sti");

    video_init();
    mm_init();
    mmap_init();
    pmm_init();
    vmm_init();

    void *tty_buffer = vmm_map((const void *)0xb8000, 1, true);

    tty_text16_init(&kernel_tty, tty_buffer, 80, 25, arch_setCursorPosition);

    kernel_tty.api.write(&kernel_tty, "Hello world!");

    while(1);
}

static void arch_setCursorPosition(tty_t *tty, int x, int y) {
    
}

void arch_disableInterrupts() {
    
}

void arch_halt() {
    
}
