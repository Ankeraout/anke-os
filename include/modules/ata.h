#ifndef __INCLUDE_MODULES_ATA_H__
#define __INCLUDE_MODULES_ATA_H__

#include <stdint.h>

enum {
    E_IOCTL_ATA_DRIVER_CREATE = 1
};

enum {
    E_IOCTL_ATA_CHANNEL_SCAN = 1
};

struct ts_ataRequestCreate {
    uint16_t a_ioPortBase;
    uint16_t a_ioPortControl;
    uint16_t a_ioPortBusMaster;
    int a_irq;
};

#endif
