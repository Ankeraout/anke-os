#include "arch/arch.h"
#include "printk.h"

void main(void) {
    arch_init();

    printk("AnkeKernel 0.1.0\n");
    printk("Compiled " __DATE__ " " __TIME__ "\n");

    while(1) {
        asm("hlt");
    }
}
