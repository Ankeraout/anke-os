#ifndef __INCLUDE_DEV_DEVICE_H__
#define __INCLUDE_DEV_DEVICE_H__

#include <stdbool.h>
#include <stdint.h>

#define C_DEVICE_MAX_ADDRESS_SIZE 4
#define C_DEVICE_MAX_IDENTIFIER_SIZE 8

enum te_deviceBus {
    E_DEVICEBUS_ROOT,
    E_DEVICEBUS_IDE,
    E_DEVICEBUS_PCI,
    E_DEVICEBUS_PS2
};

struct ts_device;

typedef int tf_deviceDriverApiInit(struct ts_device *p_device);
typedef struct ts_device tf_deviceDriverApiGetChild(
    struct ts_device *p_device,
    int p_childIndex
);
typedef int tf_deviceDriverApiGetChildCount(struct ts_device *p_device);
typedef bool tf_deviceDriverApiIsSupported(
    enum te_deviceBus p_deviceBus,
    const void *p_deviceIdentifier
);

struct ts_deviceDriver {
    const char *a_name;
    const enum te_deviceBus a_bus;
    struct {
        tf_deviceDriverApiInit *a_init;
        tf_deviceDriverApiGetChild *a_getChild;
        tf_deviceDriverApiGetChildCount *a_getChildCount;
        tf_deviceDriverApiIsSupported *a_isSupported;
    } a_api;
};

struct ts_deviceAddressCommon {
    enum te_deviceBus a_bus;
};

struct ts_deviceAddress {
    struct ts_deviceAddressCommon a_common;
    uint8_t a_address[C_DEVICE_MAX_ADDRESS_SIZE];
};

struct ts_device {
    struct ts_device *a_parent;
    struct ts_deviceAddress a_address;
    const struct ts_deviceDriver *a_driver;
    void *a_driverData;
};

int deviceRegisterDriver(const struct ts_deviceDriver *p_driver);

#endif
