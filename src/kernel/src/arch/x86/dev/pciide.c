#include "arch/x86/dev/ide.h"
#include "dev/device.h"
#include "dev/pci.h"
#include "klibc/stdlib.h"
#include "debug.h"

int pciIdeInit(struct ts_device *p_device);

const struct ts_deviceDriver g_deviceDriverPciIde = {
    .a_name = "PCI IDE controller",
    .a_api = {
        .a_init = pciIdeInit
    }
};

int pciIdeInit(struct ts_device *p_device) {
    /* TODO: PCI native mode */

    struct ts_deviceDriverDataIde *l_deviceDriverData =
        kmalloc(2 * sizeof(struct ts_deviceDriverDataIde));

    if(l_deviceDriverData == NULL) {
        debugPrint("pciide: Failed to allocate memory for IDE channel device driver data.\n");
        return 1;
    }

    struct ts_device *l_device = kmalloc(2 * sizeof(struct ts_device));

    if(l_device == NULL) {
        debugPrint("pciide: Failed to allocate memory for IDE channel device.\n");
        return 1;
    }

    l_device[0].a_parent = p_device;
    l_device[0].a_driver = &g_deviceDriverIde;
    l_device[0].a_driverData = &l_deviceDriverData[0];
    l_device[1].a_parent = p_device;
    l_device[1].a_driver = &g_deviceDriverIde;
    l_device[1].a_driverData = &l_deviceDriverData[1];
    l_deviceDriverData[0].a_ioBase = 0x1f0;
    l_deviceDriverData[0].a_ioControl = 0x3f6;
    l_deviceDriverData[0].a_ioBusMaster = 0;
    l_deviceDriverData[0].a_irq = 14;
    l_deviceDriverData[1].a_ioBase = 0x170;
    l_deviceDriverData[1].a_ioControl = 0x376;
    l_deviceDriverData[1].a_ioBusMaster = 0;
    l_deviceDriverData[1].a_irq = 15;

    for(int l_channel = 0; l_channel < 2; l_channel++) {
        l_device[l_channel].a_driver->a_api.a_init(&l_device[l_channel]);
    }

    debugPrint("pciide: Initialization complete.\n");

    return 0;
}
