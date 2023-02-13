#include "dev/device.h"
#include "arch/x86/dev/ide.h"

#include "debug.h"

static int ideInit(struct ts_device *p_device);

const struct ts_deviceDriver g_deviceDriverIde = {
    .a_name = "IDE channel controller",
    .a_init = ideInit
};

static int ideInit(struct ts_device *p_device) {
    struct ts_deviceDriverDataIde *l_deviceDriverParameters =
        (struct ts_deviceDriverDataIde *)p_device->a_driverData;

    debugPrint("ide: Initializing IDE channel (0x");
    debugPrintHex16(l_deviceDriverParameters->a_ioBase);
    debugPrint(", 0x");
    debugPrintHex16(l_deviceDriverParameters->a_ioControl);
    debugPrint(", 0x");
    debugPrintHex16(l_deviceDriverParameters->a_ioBusMaster);
    debugPrint(")...\n");
}
