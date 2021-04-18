#include "kernel/arch/x86/bios.h"
#include "kernel/arch/x86/mmap.h"
#include "kernel/arch/x86/mm/mm.h"
#include "kernel/arch/x86/mm/pmm.h"
#include "kernel/arch/x86/mm/vmm.h"

void arch_preinit();
void arch_init();

void arch_preinit() {
    bios_callContext_t context = {
        .ax = 0x1112,
        .bl = 0x00
    };

    bios_init();
    bios_call(0x10, &context);

    mm_init();
    mmap_init();
    pmm_init();
    vmm_init();
}

void arch_init() {

}
