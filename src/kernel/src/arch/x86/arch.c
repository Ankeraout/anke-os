#include <stdbool.h>
#include <stddef.h>

#include "arch/arch.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "arch/x86/dev/acpi.h"
#include "dev/device.h"
#include "klibc/stdlib.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "debug.h"

void archInit(struct ts_boot *p_boot) {
    gdtInit();
    idtInit();

    if(pmmInit(p_boot->a_memoryMap, p_boot->a_memoryMapLength) != 0) {
        debugPrint("kernel: Physical memory manager initialization failed.\n");
        debugPrint("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    struct ts_device *l_device = kmalloc(sizeof(struct ts_device));

    if(l_device == NULL) {
        debugPrint("kernel: Failed to allocate memory for root bus device.\n");
        debugPrint("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    l_device->a_driver = &g_deviceDriverAcpi;
    l_device->a_parent = NULL;

    if(l_device->a_driver->a_api.a_init(l_device)) {
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
