#include <stdbool.h>

#include "arch/x86/dev/ide.h"
#include "dev/device.h"
#include "dev/pci.h"
#include "klibc/stdlib.h"
#include "common.h"
#include "debug.h"

struct ts_pciIdeDriverData {
    struct ts_device a_channels[2];
};

static int pciIdeInit(struct ts_device *p_device);
static size_t pciIdeDriverApiGetChildCount(struct ts_device *p_device);
static struct ts_device *pciIdeDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static bool pciIdeDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
);

const struct ts_deviceDriver g_deviceDriverPciIde = {
    .a_name = "PCI IDE controller",
    .a_api = {
        .a_init = pciIdeInit,
        .a_getChild = pciIdeDriverApiGetChild,
        .a_getChildCount = pciIdeDriverApiGetChildCount,
        .a_isSupported = pciIdeDriverApiIsSupported
    }
};

static int pciIdeInit(struct ts_device *p_device) {
    /* TODO: PCI native mode */
    p_device->a_driverData = kmalloc(sizeof(struct ts_pciIdeDriverData));

    if(p_device->a_driverData == NULL) {
        debugPrint("pciide: Failed to allocate memory for driver data.\n");
        return 1;
    }

    struct ts_pciIdeDriverData *l_data =
        (struct ts_pciIdeDriverData *)p_device->a_driverData;

    struct ts_deviceDriverDataIde l_deviceDriverData;

    l_data->a_channels[0].a_parent = p_device;
    l_data->a_channels[0].a_driver = &g_deviceDriverIde;
    l_data->a_channels[0].a_driverData = &l_deviceDriverData;
    l_deviceDriverData.a_ioBase = 0x1f0;
    l_deviceDriverData.a_ioControl = 0x3f6;
    l_deviceDriverData.a_ioBusMaster = 0;
    l_deviceDriverData.a_irq = 14;
    l_data->a_channels[0].a_driver->a_api.a_init(&l_data->a_channels[0]);

    l_data->a_channels[1].a_parent = p_device;
    l_data->a_channels[1].a_driver = &g_deviceDriverIde;
    l_data->a_channels[1].a_driverData = &l_deviceDriverData;
    l_deviceDriverData.a_ioBase = 0x170;
    l_deviceDriverData.a_ioControl = 0x376;
    l_deviceDriverData.a_ioBusMaster = 0;
    l_deviceDriverData.a_irq = 15;
    l_data->a_channels[0].a_driver->a_api.a_init(&l_data->a_channels[1]);

    debugPrint("pciide: Initialization complete.\n");

    return 0;
}

static size_t pciIdeDriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 2;
}

static struct ts_device *pciIdeDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    struct ts_pciIdeDriverData *l_data =
        (struct ts_pciIdeDriverData *)p_device->a_driverData;

    return &l_data->a_channels[p_index];
}

static bool pciIdeDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
) {
    if(p_identifier->a_base.a_bus != E_DEVICEBUS_PCI) {
        return false;
    }

    const struct ts_deviceIdentifierPci *l_identifier =
        (const struct ts_deviceIdentifierPci *)p_identifier;

    return (
        (l_identifier->a_class == 0x01)
        && (l_identifier->a_subclass == 0x01)
    );
}
