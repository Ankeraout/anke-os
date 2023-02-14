#include <stdbool.h>
#include <stddef.h>

#include "arch/arch.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "arch/x86/dev/acpi.h"
#include "dev/device.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "debug.h"

static struct ts_device s_acpiDevice = {
    .a_driver = &g_deviceDriverAcpi,
    .a_parent = NULL,
    .a_driverData = NULL
};

void archInit(struct ts_boot *p_boot) {
    gdtInit();
    idtInit();

    if(pmmInit(p_boot->a_memoryMap, p_boot->a_memoryMapLength) != 0) {
        debugPrint("kernel: Physical memory manager initialization failed.\n");
        debugPrint("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    debugPrint("kernel: Allocating 32 pages...\n");
    void *l_ptr = pmmAlloc(32 * 4096);
    debugPrint("kernel: 32 pages at 0x");
    debugPrintPointer(l_ptr);
    debugPrint("\n");

    debugPrint("kernel: Freeing 32 pages...\n");
    pmmFree(l_ptr, 32 * 4096);
    debugPrint("kernel: Done!\n");

    if(s_acpiDevice.a_driver->a_init(&s_acpiDevice)) {
        debugPrint("kernel: ACPI driver initialization failed.\n");
        debugPrint("kernel: System halted.\n");
        archHaltAndCatchFire();
    }
}

void archInterruptsEnable(void) {
    sti();
}

void archInterruptsDisable(void) {
    cli();
}

void archHalt(void) {
    hlt();
}

void archHaltAndCatchFire(void) {
    while(true) {
        cli();
        hlt();
    }
}
