#include "arch/x86/dev/ide.h"
#include "dev/device.h"
#include "dev/pci.h"
#include "debug.h"

int pciIdeInit(struct ts_device *p_device);

const struct ts_deviceDriver g_deviceDriverPciIde = {
    .a_name = "PCI IDE controller",
    .a_init = pciIdeInit
};

int pciIdeInit(struct ts_device *p_device) {
    /*
    const struct ts_deviceDriverPci *l_deviceDriver =
        (const struct ts_deviceDriverPci *)p_device->a_driver;

    // TODO: PCI native mode
    uint8_t l_progIf = l_deviceDriver->a_configRead8(&p_device->a_address, 10);
    */

    struct ts_deviceDriverDataIde l_deviceDriverData[2] = {
        {
            .a_ioBase = 0x1f0,
            .a_ioControl = 0x3f6,
            .a_ioBusMaster = 0,
            .a_irq = 14
        },
        {
            .a_ioBase = 0x170,
            .a_ioControl = 0x376,
            .a_ioBusMaster = 0,
            .a_irq = 15
        }
    };

    struct ts_device l_device[2] = {
        {
            .a_parent = p_device,
            .a_driver = &g_deviceDriverIde,
            .a_driverData = &l_deviceDriverData[0]
        },
        {
            .a_parent = p_device,
            .a_driver = &g_deviceDriverIde,
            .a_driverData = &l_deviceDriverData[1]
        }
    };

    for(int l_channel = 0; l_channel < 2; l_channel++) {
        l_device[l_channel].a_driver->a_init(&l_device[l_channel]);
    }

    debugPrint("pciide: Initialization complete.\n");

    return 0;
}
