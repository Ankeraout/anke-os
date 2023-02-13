#include "dev/device.h"
#include "debug.h"

int pciIdeInit(struct ts_device *p_device);

const struct ts_deviceDriver g_deviceDriverPciIde = {
    .a_name = "PCI IDE controller",
    .a_init = pciIdeInit
};

int pciIdeInit(struct ts_device *p_device) {
    debugPrint("pciide: init()\n");
}
