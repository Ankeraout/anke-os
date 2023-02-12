#ifndef __INCLUDE_DEV_DEVICE_H__
#define __INCLUDE_DEV_DEVICE_H__

#include <stdint.h>

struct ts_device;

typedef int tf_deviceDriverFuncInit(struct ts_device *p_device);

struct ts_deviceDriver {
    tf_deviceDriverFuncInit *a_init;
};

union tu_deviceAddress {
    struct {
        uint8_t a_bus;
        uint8_t a_slot;
        uint8_t a_func;
    } a_pci;

    struct {
        int a_drive;
    } a_ide;

    struct {
        int a_port;
    } a_ps2;
};

struct ts_device {
    struct ts_device *a_parent;
    union tu_deviceAddress a_address;
    const struct ts_deviceDriver *a_driver;
    void *a_driverData;
};

#endif
