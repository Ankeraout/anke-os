#include "kernel/arch/x86/bios.h"

void arch_preinit();
void arch_init();

void arch_preinit() {
    bios_callContext_t context = {
        .ax = 0x1112,
        .bl = 0x00
    };

    bios_init();
    bios_call(0x10, &context);
}

void arch_init() {

}
