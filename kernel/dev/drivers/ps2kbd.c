#include <stdbool.h>
#include <stdint.h>

#include <kernel/dev/ps2.h>
#include <kernel/common.h>
#include <kernel/debug.h>

#define C_PS2_CMD_ECHO 0xee
#define C_PS2_CMD_IDENTIFY 0xf2
#define C_PS2_CMD_SCAN_ENABLE 0xf4
#define C_PS2_CMD_SCAN_DISABLE 0xf5
#define C_PS2_CMD_RESEND 0xfe
#define C_PS2_CMD_RESET 0xff

static int ps2KbdDeviceDriverApiInit(struct ts_device *p_device);
static struct ts_device *ps2KbdDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static size_t ps2KbdDeviceDriverApiGetChildCount(struct ts_device *p_device);
static bool ps2KbdDeviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
);
static void ps2KbdDeviceDriverApiReceive(
    struct ts_device *p_device,
    uint8_t p_value
);

const struct ts_deviceDriverPs2Device g_deviceDriverPs2Kbd = {
    .a_base = {
        .a_name = "PS/2 keyboard",
        .a_bus = E_DEVICEBUS_PS2,
        .a_api = {
            .a_init = ps2KbdDeviceDriverApiInit,
            .a_getChild = ps2KbdDeviceDriverApiGetChild,
            .a_getChildCount = ps2KbdDeviceDriverApiGetChildCount,
            .a_isSupported = ps2KbdDeviceDriverApiIsSupported
        }
    },
    .a_api = {
        .a_receive = ps2KbdDeviceDriverApiReceive
    }
};

static int ps2KbdDeviceDriverApiInit(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    struct ts_device *l_port = p_device->a_parent;
    const struct ts_deviceDriverPs2Port *l_portDriver =
        (const struct ts_deviceDriverPs2Port *)l_port->a_driver;

    l_portDriver->a_api.a_send(l_port, C_PS2_CMD_SCAN_ENABLE);

    return 0;
}

static struct ts_device *ps2KbdDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_index);

    return NULL;
}

static size_t ps2KbdDeviceDriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static bool ps2KbdDeviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
) {
    if(p_identifier->a_base.a_bus != E_DEVICEBUS_PS2) {
        return false;
    }

    const struct ts_deviceIdentifierPs2 *l_identifier =
        (const struct ts_deviceIdentifierPs2 *)p_identifier;

    if(l_identifier->a_identifier[0] == 2) {
        if(l_identifier->a_identifier[1] == 0xab) {
            if(
                (l_identifier->a_identifier[2] == 0x83)
                || (l_identifier->a_identifier[2] == 0xc1)
            ) {
                return true;
            }
        }
    }

    return false;
}

static void ps2KbdDeviceDriverApiReceive(
    struct ts_device *p_device,
    uint8_t p_value
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_value);
}
