#ifndef __INCLUDE_ARCH_X86_DEV_IDE_H__
#define __INCLUDE_ARCH_X86_DEV_IDE_H__

#include "dev/device.h"

struct ts_deviceDriverDataIde {
    uint16_t a_ioBase;
    uint16_t a_ioControl;
    uint16_t a_ioBusMaster;
    int a_irq;
};

extern const struct ts_deviceDriver g_deviceDriverIde;

#endif
