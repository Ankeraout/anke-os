#include "arch/i686/bioscall.h"
#include "arch/i686/mm/mm.h"
#include "arch/i686/video.h"

#include "libk/stdio.h"

void arch_init();
void arch_disableInterrupts();
void arch_halt();

void arch_init() {
    bioscall_init();
    video_init();

    char buffer[4096];

    sprintf(buffer, "%#08x\n", 0xdeadbeef);

    video_puts(buffer);

    while(1);
}

void arch_disableInterrupts() {
    
}

void arch_halt() {
    
}
