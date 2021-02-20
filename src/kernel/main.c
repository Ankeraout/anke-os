#include <stdbool.h>

#include "time.h"
#include "arch/arch.h"
#include "dev/disk/disk.h"
#include "libk/stdio.h"

void kernel_main() {
    arch_preinit();

    printf("Welcome to AnkeOS!\n");

    disk_init();

    arch_init();

    printf("Initialization done.\n");

    while(true) {
        arch_halt();
    }
}
