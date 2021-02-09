#include <stdbool.h>

#include "time.h"
#include "arch/arch.h"
#include "libk/stdio.h"

void kernel_main() {
    arch_init();

    printf("Welcome to AnkeOS!\n");

    while(true) {
        arch_halt();
    }
}
