#include <stdbool.h>
#include <stdint.h>

#include "dev/ps2.h"
#include "common.h"
#include "debug.h"

static int ps2MouseDeviceDriverApiInit(struct ts_device *p_device);
static struct ts_device *ps2MouseDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static size_t ps2MouseDeviceDriverApiGetChildCount(struct ts_device *p_device);
static bool ps2MouseDeviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
);
static void ps2MouseDeviceDriverApiReceive(
    struct ts_device *p_device,
    uint8_t p_value
);

const struct ts_deviceDriverPs2Device g_deviceDriverPs2Mouse = {
    .a_base = {
        .a_name = "PS/2 mouse",
        .a_bus = E_DEVICEBUS_PS2,
        .a_api = {
            .a_init = ps2MouseDeviceDriverApiInit,
            .a_getChild = ps2MouseDeviceDriverApiGetChild,
            .a_getChildCount = ps2MouseDeviceDriverApiGetChildCount,
            .a_isSupported = ps2MouseDeviceDriverApiIsSupported
        }
    },
    .a_api = {
        .a_receive = ps2MouseDeviceDriverApiReceive
    }
};

static int ps2MouseDeviceDriverApiInit(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static struct ts_device *ps2MouseDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_index);

    return NULL;
}

static size_t ps2MouseDeviceDriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static bool ps2MouseDeviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
) {
    if(p_identifier->a_base.a_bus != E_DEVICEBUS_PS2) {
        return false;
    }

    const struct ts_deviceIdentifierPs2 *l_identifier =
        (const struct ts_deviceIdentifierPs2 *)p_identifier;

    if(l_identifier->a_identifier[0] == 1) {
        if(l_identifier->a_identifier[1] == 0x00) {
            return true;
        }
    }

    return false;
}

static void ps2MouseDeviceDriverApiReceive(
    struct ts_device *p_device,
    uint8_t p_value
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_value);
}
