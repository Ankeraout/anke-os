#ifndef __INCLUDE_DEV_PS2_H__
#define __INCLUDE_DEV_PS2_H__

#include <stdbool.h>
#include <stdint.h>

#include "dev/device.h"

enum te_deviceAddressPs2 {
    E_DEVICEADDRESS_PS2_0,
    E_DEVICEADDRESS_PS2_1
};

struct ts_deviceDriverPs2Controller {
    struct ts_deviceDriver a_base;
    struct {
        void (*a_send)(
            struct ts_device *p_device,
            enum te_deviceAddressPs2 p_port,
            uint8_t p_value
        );
    } a_api;
};

struct ts_deviceDriverPs2Port {
    struct ts_deviceDriver a_base;
    struct {
        void (*a_detect)(struct ts_device *p_device);
        void (*a_send)(struct ts_device *p_device, uint8_t p_value);
        void (*a_receive)(struct ts_device *p_device, uint8_t p_value);
    } a_api;
};

struct ts_deviceDriverPs2Device {
    struct ts_deviceDriver a_base;
    struct {
        void (*a_receive)(struct ts_device *p_device, uint8_t p_value);
    } a_api;
};

struct ts_deviceIdentifierPs2 {
    struct ts_deviceIdentifierCommon a_base;
    uint8_t a_identifier[3];
};

#endif
