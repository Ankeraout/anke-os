#include <stdbool.h>
#include <stddef.h>

#include "arch/arch.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "arch/x86/dev/drivers/acpi.h"
#include "arch/x86/dev/drivers/pciide.h"
#include "dev/device.h"
#include "klibc/stdlib.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "debug.h"

static void archShowDeviceTree(struct ts_device *p_device, size_t p_level);

int archPreinit(struct ts_boot *p_boot) {
    gdtInit();
    idtInit();

    if(pmmInit(p_boot->a_memoryMap, p_boot->a_memoryMapLength) != 0) {
        debugPrint("kernel: Physical memory manager initialization failed.\n");
        return 1;
    }

    return 0;
}

int archInit(void) {
    // Register PCI drivers
    deviceRegisterDriver(&g_deviceDriverPciIde);

    struct ts_device *l_device = kmalloc(sizeof(struct ts_device));

    if(l_device == NULL) {
        debugPrint("kernel: Failed to allocate memory for root bus device.\n");
        return 1;
    }

    l_device->a_driver = &g_deviceDriverAcpi;
    l_device->a_parent = NULL;

    if(l_device->a_driver->a_api.a_init(l_device)) {
        debugPrint("kernel: ACPI driver initialization failed.\n");
        return 1;
    }

    debugPrint("kernel: Device tree:\n");
    archShowDeviceTree(l_device, 0);

    return 0;
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

static void archShowDeviceTree(struct ts_device *p_device, size_t p_level) {
    for(size_t l_level = 0; l_level < p_level; l_level++) {
        debugPrint("    ");
    }

    debugPrint("- ");
    debugPrint(p_device->a_driver->a_name);
    debugPrint("\n");

    size_t l_childCount = p_device->a_driver->a_api.a_getChildCount(p_device);

    for(size_t l_index = 0; l_index < l_childCount; l_index++) {
        archShowDeviceTree(
            p_device->a_driver->a_api.a_getChild(p_device, l_index),
            p_level + 1
        );
    }
}
