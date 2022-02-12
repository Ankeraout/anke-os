// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/asm.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/mmu.h"
#include "arch/x86/multiboot.h"

// =============================================================================
// Private constants definition
// =============================================================================
/**
 * @brief This constant defines the magic number of the Multiboot2 standard.
 *        If the kernel was loaded by a Multiboot2-compliant boot loader, the
 *        EAX register contains this value when the executable is started.
 */
#define C_MULTIBOOT2_MAGIC 0x36d76289

// =============================================================================
// Public functions declaration
// =============================================================================
/**
 * @brief This function is the entry point of the C code of the kernel.
 *
 * @param[in] p_eax The value of the EAX register when the kernel was started.
 * @param[in] p_ebx The value of the EBX register when the kernel was started.
 */
void kernelMain(uint32_t p_eax, uint32_t p_ebx);

// =============================================================================
// Public functions definition
// =============================================================================
void kernelMain(uint32_t p_eax, uint32_t p_ebx) {
    // Initialize GDT
    gdtInit();

    // Initialize IDT
    idtInit();

    if(p_eax != C_MULTIBOOT2_MAGIC) {
        // Stop boot process

        while(true) {
            cli();
            hlt();
        }
    }

    // Initialize MMU
    mmuInit();

    // Initialize Multiboot
    multibootInit((const void *)p_ebx);

    // Idle loop
    while(true) {
        cli();
        hlt();
    }
}
