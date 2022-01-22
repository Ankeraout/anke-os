#include <stdbool.h>

#include "arch/x86/gdt.h"

void kernelMain(void) {
    // Initialize GDT
    gdtInit();

    while(true) {

    }
}
