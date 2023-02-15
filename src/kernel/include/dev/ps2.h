#ifndef __INCLUDE_DEV_PS2_H__
#define __INCLUDE_DEV_PS2_H__

#include <stdbool.h>
#include <stdint.h>

#include "dev/device.h"

typedef bool tf_ps2CanReceive(struct ts_device *p_device, int p_port);
typedef uint8_t tf_ps2Receive(struct ts_device *p_device, int p_port);
typedef void tf_ps2Send(
    struct ts_device *p_device,
    int p_port,
    uint8_t p_value
);

struct ts_deviceDriverPs2 {
    struct ts_deviceDriver a_base;
    struct {
        tf_ps2CanReceive *a_canReceive;
        tf_ps2Receive *a_receive;
        tf_ps2Send *a_send;
    } a_api;
};

enum te_deviceAddressPs2 {
    E_DEVICEADDRESS_PS2_0,
    E_DEVICEADDRESS_PS2_1
};

struct ts_deviceIdentifierPs2 {
    struct ts_deviceIdentifierCommon a_base;
    uint8_t l_identifier;
};

#endif
