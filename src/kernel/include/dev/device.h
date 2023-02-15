#ifndef __INCLUDE_DEV_DEVICE_H__
#define __INCLUDE_DEV_DEVICE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define C_DEVICE_MAX_ADDRESS_SIZE 16

enum te_deviceBus {
    E_DEVICEBUS_ANY,
    E_DEVICEBUS_ROOT,
    E_DEVICEBUS_IDE,
    E_DEVICEBUS_PCI,
    E_DEVICEBUS_PS2
};

struct ts_deviceIdentifierCommon {
    enum te_deviceBus a_bus;
};

struct ts_deviceIdentifier {
    struct ts_deviceIdentifierCommon a_base;
    uint8_t a_identifier;
};

struct ts_device;

typedef int tf_deviceDriverApiInit(struct ts_device *p_device);
typedef struct ts_device *tf_deviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_childIndex
);
typedef size_t tf_deviceDriverApiGetChildCount(struct ts_device *p_device);
typedef bool tf_deviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_deviceIdentifier
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

const struct ts_deviceDriver *deviceGetDriver(
    const struct ts_deviceIdentifier *p_deviceIdentifier
);
int deviceRegisterDriver(const struct ts_deviceDriver *p_driver);

#endif
