#include "arch/arch.h"
#include "arch/amd64/gdt.h"
#include "printk.h"

void main(void) {
    arch_init();

    printk("Hello, World!\n");

    while(1) {
        asm("hlt");
    }
}
