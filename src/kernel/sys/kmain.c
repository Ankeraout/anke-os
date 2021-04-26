#include "kernel/arch/arch.h"

#include "kernel/libk/stdio.h"

void kmain() {
    arch_preinit();

    puts("Welcome to AnkeOS!\n");

    while(1) {
        
    }
}
