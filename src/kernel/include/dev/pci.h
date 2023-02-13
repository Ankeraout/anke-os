#ifndef __INCLUDE_DEV_PCI_H__
#define __INCLUDE_DEV_PCI_H__

#include <stdbool.h>
#include <stdint.h>

#include "dev/device.h"

typedef uint8_t tf_pciConfigRead8(
    const union tu_deviceAddress *p_deviceAddress,
    uint8_t p_offset
);
typedef uint16_t tf_pciConfigRead16(
    const union tu_deviceAddress *p_deviceAddress,
    uint8_t p_offset
);
typedef uint32_t tf_pciConfigRead32(
    const union tu_deviceAddress *p_deviceAddress,
    uint8_t p_offset
);
typedef void tf_pciConfigWrite8(
    const union tu_deviceAddress *p_deviceAddress,
    uint8_t p_offset,
    uint8_t p_value
);
typedef void tf_pciConfigWrite16(
    const union tu_deviceAddress *p_deviceAddress,
    uint8_t p_offset,
    uint16_t p_value
);
typedef void tf_pciConfigWrite32(
    const union tu_deviceAddress *p_deviceAddress,
    uint8_t p_offset,
    uint32_t p_value
);

struct ts_deviceDriverPci {
    struct ts_deviceDriver a_driver;
    tf_pciConfigRead8 *a_configRead8;
    tf_pciConfigRead16 *a_configRead16;
    tf_pciConfigRead32 *a_configRead32;
    tf_pciConfigWrite8 *a_configWrite8;
    tf_pciConfigWrite16 *a_configWrite16;
    tf_pciConfigWrite32 *a_configWrite32;
};

#endif
