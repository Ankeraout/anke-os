#include "arch/i686/bioscall.h"

void arch_init();
void arch_disableInterrupts();
void arch_halt();

void arch_init() {
    bioscall_init();

    bioscall_context_t context = {
        .ah = 0x0e,
        .al = '!',
        .bh = 0,
        .bl = 0x07
    };

    bioscall(&context, 0x10);

    while(1);
}

void arch_disableInterrupts() {
    
}

void arch_halt() {
    
}
