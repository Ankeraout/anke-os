#ifndef __INCLUDE_DEV_PCI_H__
#define __INCLUDE_DEV_PCI_H__

#include <stdbool.h>
#include <stdint.h>

#include "dev/device.h"
#include "dev/pci.h"

struct ts_deviceAddressPci {
    struct ts_deviceAddressCommon a_common;
    uint8_t a_bus;
    uint8_t a_slot;
    uint8_t a_func;
};

struct ts_deviceIdentifierPci {
    uint16_t a_vendor;
    uint16_t a_device;
    uint8_t a_class;
    uint8_t a_subclass;
    uint8_t a_programmingInterface;
    uint8_t a_revision;
};

typedef uint8_t tf_pciConfigRead8(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
typedef uint16_t tf_pciConfigRead16(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
typedef uint32_t tf_pciConfigRead32(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
typedef void tf_pciConfigWrite8(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint8_t p_value
);
typedef void tf_pciConfigWrite16(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint16_t p_value
);
typedef void tf_pciConfigWrite32(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint32_t p_value
);

struct ts_deviceDriverPci {
    struct ts_deviceDriver a_base;
    struct {
        tf_pciConfigRead8 *a_configRead8;
        tf_pciConfigRead16 *a_configRead16;
        tf_pciConfigRead32 *a_configRead32;
        tf_pciConfigWrite8 *a_configWrite8;
        tf_pciConfigWrite16 *a_configWrite16;
        tf_pciConfigWrite32 *a_configWrite32;
    } a_api;
};

#endif
