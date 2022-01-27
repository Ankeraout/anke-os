// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/asm.h"
#include "arch/x86/gdt.h"
#include "arch/x86/mmu.h"

// =============================================================================
// Private constants definition
// =============================================================================
#define C_MULTIBOOT2_MAGIC 0x36d76289

// =============================================================================
// Public functions definition
// =============================================================================
void kernelMain(uint32_t p_eax, uint32_t p_ebx) {
    // Initialize GDT
    gdtInit();

    if(p_eax != C_MULTIBOOT2_MAGIC) {
        // Stop boot process

        while(true) {
            cli();
            hlt();
        }
    }

    // Initialize MMU
    mmuInit();

    while(true) {
        
    }
}
