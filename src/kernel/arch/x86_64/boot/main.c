#include "kernel/printk.h"

void _start(void) {
    printk("AnkeOS kernel\n");

    while(1) {
        asm("hlt");
    }
}
