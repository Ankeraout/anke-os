#ifndef __INCLUDE_DEV_IDE_H__
#define __INCLUDE_DEV_IDE_H__

#include <stdint.h>

enum te_deviceAddressIde {
    E_DEVICEADDRESS_IDE_MASTER,
    E_DEVICEADDRESS_IDE_SLAVE
};

struct ts_deviceIdentifierIde {
    uint8_t l_identifier[2];
};

#endif
