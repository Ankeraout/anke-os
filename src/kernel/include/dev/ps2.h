#ifndef __INCLUDE_DEV_PS2_H__
#define __INCLUDE_DEV_PS2_H__

#include <stdbool.h>
#include <stdint.h>

#include "dev/device.h"

typedef bool tf_deviceDriverPs2ControllerFuncCanReceive(struct ts_device *p_device, int p_port);
typedef uint8_t tf_deviceDriverPs2ControllerReceive(struct ts_device *p_device, int p_port);
typedef void tf_deviceDriverPs2ControllerSend(
    struct ts_device *p_device,
    int p_port,
    uint8_t p_value
);
typedef void tf_deviceDriverPs2DeviceInterrupt(struct ts_device *p_device);

struct ts_deviceDriverPs2Controller {
    struct ts_deviceDriver a_base;
    struct {
        tf_deviceDriverPs2ControllerFuncCanReceive *a_canReceive;
        tf_deviceDriverPs2ControllerReceive *a_receive;
        tf_deviceDriverPs2ControllerSend *a_send;
    } a_api;
};

struct ts_deviceDriverPs2Device {
    struct ts_deviceDriver a_base;
    struct {
        tf_deviceDriverPs2DeviceInterrupt *a_interrupt;
    } a_api;
};

enum te_deviceAddressPs2 {
    E_DEVICEADDRESS_PS2_0,
    E_DEVICEADDRESS_PS2_1
};

struct ts_deviceIdentifierPs2 {
    struct ts_deviceIdentifierCommon a_base;
    uint8_t l_identifier[3];
};

#endif
