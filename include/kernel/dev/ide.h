#ifndef __INCLUDE_DEV_IDE_H__
#define __INCLUDE_DEV_IDE_H__

#include <stdint.h>

#include <kernel/dev/device.h>

enum te_deviceAddressIde {
    E_DEVICEADDRESS_IDE_MASTER,
    E_DEVICEADDRESS_IDE_SLAVE
};

struct ts_deviceAddressIde {
    struct ts_deviceAddressCommon a_base;
    enum te_deviceAddressIde a_address;
};

struct ts_deviceIdentifierIde {
    struct ts_deviceIdentifier a_base;
    uint8_t l_identifier[2];
};

#endif
