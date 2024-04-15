#include <stdint.h>

#include "asm.h"
#include "stdio.h"

int main() {
    stdio_init();

    puts("AnkeOS bootloader 0.1.0 (long mode bootstrap)\n");

    while(1) {
        cli();
        hlt();
    }

    return 0;
}
