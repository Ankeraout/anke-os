#include <stdbool.h>

#include "time.h"
#include "arch/arch.h"
#include "libk/stdio.h"

void kernel_main() {
    arch_init();

    while(true) {
        arch_halt();
    }
}
