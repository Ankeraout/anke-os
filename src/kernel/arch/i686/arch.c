#include "arch/i686/bioscall.h"
#include "arch/i686/idt.h"
#include "arch/i686/mmap.h"
#include "arch/i686/pic.h"
#include "arch/i686/video.h"
#include "arch/i686/mm/mm.h"
#include "arch/i686/mm/pmm.h"
#include "arch/i686/mm/vmm.h"

#include "libk/stdio.h"

void arch_init();
void arch_disableInterrupts();
void arch_halt();

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

    while(1);
}

void arch_disableInterrupts() {
    
}

void arch_halt() {
    
}
