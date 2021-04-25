#include "kernel/arch/arch.h"

#include "kernel/libk/stdio.h"

void kmain() {
    arch_preinit();

    puts("Hello world!\n");

    while(1) {
        
    }
}
