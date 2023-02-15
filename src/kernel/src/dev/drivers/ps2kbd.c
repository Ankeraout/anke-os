#include <stdbool.h>
#include <stdint.h>

#include "dev/ps2.h"
#include "common.h"
#include "debug.h"

static int ps2KbdDeviceDriverApiInit(struct ts_device *p_device);
static struct ts_device *ps2KbdDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static size_t ps2KbdDeviceDriverApiGetChildCount(struct ts_device *p_device);
static bool ps2KbdDeviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
);
static void ps2KbdDeviceDriverApiInterrupt(struct ts_device *p_device);

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
        .a_interrupt = ps2KbdDeviceDriverApiInterrupt
    }
};

static int ps2KbdDeviceDriverApiInit(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    const struct ts_deviceDriverPs2Controller *l_controllerDriver =
        (const struct ts_deviceDriverPs2Controller *)p_device->a_parent->a_driver;
    
    l_controllerDriver->a_api.a_send(
        p_device->a_parent,
        *((enum te_deviceAddressPs2 *)p_device->a_address.a_address),
        0xf4
    );

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

static void ps2KbdDeviceDriverApiInterrupt(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    const struct ts_deviceDriverPs2Controller *l_controllerDriver =
        (const struct ts_deviceDriverPs2Controller *)p_device->a_parent->a_driver;

    const enum te_deviceAddressPs2 l_deviceAddress =
        *(const enum te_deviceAddressPs2 *)p_device->a_address.a_address;

    while(
        l_controllerDriver->a_api.a_canReceive(
            p_device->a_parent,
            l_deviceAddress
        )
    ) {
        debugPrint("ps2kbd: 0x");
        
        uint8_t l_data = l_controllerDriver->a_api.a_receive(
            p_device->a_parent, l_deviceAddress
        );

        debugPrintHex8(l_data);
        debugPrint("\n");
    }
}
