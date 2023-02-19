#include <stdbool.h>
#include <stdint.h>

#include <kernel/arch/arch.h>
#include <kernel/dev/ps2.h>
#include <kernel/dev/timer.h>
#include <kernel/dev/drivers/unknown.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/klibc/string.h>
#include <kernel/common.h>
#include <kernel/debug.h>

#define C_PS2_RESPONSE_SELF_TEST_PASSED 0xaa
#define C_PS2_RESPONSE_ECHO 0xee
#define C_PS2_RESPONSE_ACK 0xfa
#define C_PS2_RESPONSE_SELF_TEST_FAILED_1 0xfc
#define C_PS2_RESPONSE_SELF_TEST_FAILED_2 0xfd
#define C_PS2_RESPONSE_RESEND 0xfe
#define C_PS2_CMD_ECHO 0xee
#define C_PS2_CMD_IDENTIFY 0xf2
#define C_PS2_CMD_SCAN_ENABLE 0xf4
#define C_PS2_CMD_SCAN_DISABLE 0xf5
#define C_PS2_CMD_RESEND 0xfe
#define C_PS2_CMD_RESET 0xff

#define C_PS2_RECEIVE_BUFFER_SIZE 16
#define C_PS2_TIMEOUT_RESET 1000
#define C_PS2_TIMEOUT_ACK 100
#define C_PS2_TIMEOUT_IDENTIFY 100
#define C_PS2_MAX_RESEND_COUNT 3

static bool ps2PortWaitSelfTest(struct ts_device *p_device);
static bool ps2PortWaitAck(struct ts_device *p_device);
static int ps2PortDeviceDriverApiInit(struct ts_device *p_device);
static struct ts_device *Ps2PortDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static size_t ps2PortDeviceDriverApiGetChildCount(struct ts_device *p_device);
static bool ps2PortDeviceDriverApiIsSupported(
    const struct ts_deviceIdentifier *p_identifier
);
static void ps2PortDeviceDriverApiDetect(struct ts_device *p_device);
static void ps2PortDeviceDriverApiReceive(
    struct ts_device *p_device,
    uint8_t p_value
);
static void ps2PortDeviceDriverApiSend(
    struct ts_device *p_device,
    uint8_t p_value
);
static uint8_t ps2PortReadBuffer(
    struct ts_device *p_device
);

struct ts_deviceDriverDataPs2Port {
    uint8_t a_receiveBuffer[C_PS2_RECEIVE_BUFFER_SIZE];
    int a_receiveBufferLength;
    int a_receiveBufferReadIndex;
    int a_receiveBufferWriteIndex;
    bool a_connected;
    bool a_driverFound;
    bool a_receivedSelfTest;
    bool a_receivedEcho;
    bool a_receivedAck;
    bool a_expectingSelfTest;
    bool a_expectingAck;
    bool a_selfTestPassed;
    uint8_t a_lastSentByte;
    int a_resendCount;
    struct ts_device a_device;
};

const struct ts_deviceDriverPs2Port g_deviceDriverPs2Port = {
    .a_base = {
        .a_name = "PS/2 port",
        .a_bus = E_DEVICEBUS_PS2,
        .a_api = {
            .a_init = ps2PortDeviceDriverApiInit,
            .a_getChild = Ps2PortDeviceDriverApiGetChild,
            .a_getChildCount = ps2PortDeviceDriverApiGetChildCount,
            .a_isSupported = ps2PortDeviceDriverApiIsSupported
        }
    },
    .a_api = {
        .a_detect = ps2PortDeviceDriverApiDetect,
        .a_receive = ps2PortDeviceDriverApiReceive,
        .a_send = ps2PortDeviceDriverApiSend
    }
};


static bool ps2PortWaitSelfTest(struct ts_device *p_device) {
    uint64_t l_startTime = timerGetTime();
    uint64_t l_endTime = l_startTime + C_PS2_TIMEOUT_RESET;

    volatile struct ts_deviceDriverDataPs2Port *l_deviceData =
        (struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    while(
        (timerGetTime() < l_endTime)
        && (!l_deviceData->a_receivedSelfTest)
    ) {
        archHalt();
    }

    l_deviceData->a_expectingSelfTest = false;

    return l_deviceData->a_receivedSelfTest;
}

static bool ps2PortWaitAck(struct ts_device *p_device) {
    uint64_t l_startTime = timerGetTime();
    uint64_t l_endTime = l_startTime + C_PS2_TIMEOUT_ACK;

    volatile struct ts_deviceDriverDataPs2Port *l_deviceData =
        (struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    while(
        (timerGetTime() < l_endTime)
        && (!l_deviceData->a_receivedAck)
    ) {
        archHalt();
    }

    l_deviceData->a_expectingAck = false;

    return l_deviceData->a_receivedAck;
}

static int ps2PortDeviceDriverApiInit(struct ts_device *p_device) {
    p_device->a_driverData = kmalloc(sizeof(struct ts_deviceDriverDataPs2Port));

    if(p_device->a_driverData == NULL) {
        debug("ps2port: Failed to allocate memory for driver data.\n");
        return 1;
    }

    return 0;
}

static struct ts_device *Ps2PortDeviceDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    if(p_index != 0) {
        return NULL;
    }

    volatile struct ts_deviceDriverDataPs2Port *l_deviceData =
        (struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    if(l_deviceData->a_connected) {
        return (struct ts_device *)&l_deviceData->a_device;
    }

    return NULL;
}

static size_t ps2PortDeviceDriverApiGetChildCount(struct ts_device *p_device) {
    volatile struct ts_deviceDriverDataPs2Port *l_deviceData =
        (struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    if(l_deviceData->a_connected) {
        return 1;
    }

    return 0;
}

static bool ps2PortDeviceDriverApiIsSupported(
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

static void ps2PortDeviceDriverApiDetect(struct ts_device *p_device) {
    // Initialize port structure
    volatile struct ts_deviceDriverDataPs2Port *l_driverData =
        (volatile struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    l_driverData->a_connected = false;
    l_driverData->a_driverFound = false;
    l_driverData->a_receivedAck = false;
    l_driverData->a_receivedEcho = false;
    l_driverData->a_receivedSelfTest = false;

    // Send reset command
    ps2PortDeviceDriverApiSend(p_device, C_PS2_CMD_RESET);

    if(!ps2PortWaitSelfTest(p_device)) {
        debug("Device self-test timed out.\n");
        return;
    }

    if(!l_driverData->a_selfTestPassed) {
        debug("ps2port: Device self-test failed.\n");
        return;
    }

    // Disable scanning
    ps2PortDeviceDriverApiSend(p_device, C_PS2_CMD_SCAN_DISABLE);

    if(!ps2PortWaitAck(p_device)) {
        debug("ps2port: Device ACK timed out.\n");
        return;
    }

    // Flush receive buffer
    l_driverData->a_receiveBufferLength = 0;
    l_driverData->a_receiveBufferReadIndex = 0;
    l_driverData->a_receiveBufferWriteIndex = 0;

    // Identify device
    ps2PortDeviceDriverApiSend(p_device, C_PS2_CMD_IDENTIFY);

    if(!ps2PortWaitAck(p_device)) {
        debug("ps2port: Device ACK timed out.\n");
        return;
    }

    // Give the device some time to respond
    timerSleep(C_PS2_TIMEOUT_IDENTIFY);

    // Read identification data
    const int l_identifierLength = l_driverData->a_receiveBufferLength;

    if(l_identifierLength > 2) {
        return;
    }

    struct ts_deviceIdentifierPs2 l_identifier = {
        .a_base = {
            .a_bus = E_DEVICEBUS_PS2
        }
    };

    l_identifier.a_identifier[0] = l_identifierLength;

    for(int l_index = 0; l_index < l_identifierLength; l_index++) {
        l_identifier.a_identifier[l_index + 1] = ps2PortReadBuffer(p_device);
    }

    // Find a driver for the device
    l_driverData->a_device.a_driver = deviceGetDriver(
        (const struct ts_deviceIdentifier *)&l_identifier
    );

    l_driverData->a_device.a_parent = p_device;
    l_driverData->a_connected = true;
    l_driverData->a_driverFound =
        l_driverData->a_device.a_driver != &g_deviceDriverUnknown;

    if(l_driverData->a_driverFound) {
        if(
            l_driverData->a_device.a_driver->a_api.a_init(
                (struct ts_device *)&l_driverData->a_device
            ) != 0
        ) {
            l_driverData->a_driverFound = false;
        }
    }
}

static void ps2PortDeviceDriverApiReceive(
    struct ts_device *p_device,
    uint8_t p_value
) {
    volatile struct ts_deviceDriverDataPs2Port *l_driverData =
        (volatile struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    if(p_value == C_PS2_RESPONSE_SELF_TEST_PASSED) {
        l_driverData->a_receivedSelfTest = true;
        l_driverData->a_expectingSelfTest = false;
        l_driverData->a_selfTestPassed = true;
    } else if(p_value == C_PS2_RESPONSE_ECHO) {
        l_driverData->a_receivedEcho = true;
    } else if(p_value == C_PS2_RESPONSE_ACK) {
        l_driverData->a_expectingAck = false;
        l_driverData->a_receivedAck = true;
    } else if(
        (p_value == C_PS2_RESPONSE_SELF_TEST_FAILED_1)
        || (p_value == C_PS2_RESPONSE_SELF_TEST_FAILED_2)
    ) {
        l_driverData->a_connected = false;
        l_driverData->a_receivedSelfTest = true;
        l_driverData->a_expectingSelfTest = false;
        l_driverData->a_selfTestPassed = false;
    } else if(p_value == C_PS2_RESPONSE_RESEND) {
        l_driverData->a_expectingAck = false;
        l_driverData->a_resendCount++;

        if(l_driverData->a_resendCount == C_PS2_MAX_RESEND_COUNT) {
            // Abort
            l_driverData->a_connected = false;
        } else {
            ps2PortDeviceDriverApiSend(p_device, l_driverData->a_lastSentByte);
        }
    } else {
        if(l_driverData->a_driverFound) {
            const struct ts_deviceDriverPs2Device *l_deviceDriver =
                (const struct ts_deviceDriverPs2Device *)l_driverData->a_device.a_driver;

            l_deviceDriver->a_api.a_receive(
                (struct ts_device *)&l_driverData->a_device,
                p_value
            );
        } else if(l_driverData->a_receiveBufferLength != C_PS2_RECEIVE_BUFFER_SIZE) {
            l_driverData->a_receiveBuffer[l_driverData->a_receiveBufferWriteIndex++] = p_value;

            if(l_driverData->a_receiveBufferWriteIndex == C_PS2_RECEIVE_BUFFER_SIZE) {
                l_driverData->a_receiveBufferWriteIndex = 0;
            }

            l_driverData->a_receiveBufferLength++;
        }
    }
}

static void ps2PortDeviceDriverApiSend(
    struct ts_device *p_device,
    uint8_t p_value
) {
    volatile struct ts_deviceDriverDataPs2Port *l_driverData =
        (volatile struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;
    const struct ts_deviceDriverPs2Controller *l_controllerDriver =
        (const struct ts_deviceDriverPs2Controller *)p_device->a_parent->a_driver;
    const enum te_deviceAddressPs2 *l_deviceAddress =
        (const enum te_deviceAddressPs2 *)p_device->a_address.a_address;

    while(true) {
        bool l_readyToSend = (
            (!l_driverData->a_expectingAck)
            && (!l_driverData->a_expectingSelfTest)
        );

        if(l_readyToSend) {
            break;
        }

        archHalt();
    }

    if(p_value == C_PS2_CMD_RESET) {
        l_driverData->a_receivedSelfTest = false;
        l_driverData->a_expectingSelfTest = true;
    } else if((p_value != C_PS2_CMD_ECHO) && (p_value != C_PS2_CMD_RESEND)) {
        l_driverData->a_expectingAck = true;
        l_driverData->a_receivedAck = false;
    }

    if(p_value != C_PS2_CMD_RESEND) {
        l_driverData->a_lastSentByte = p_value;
        l_driverData->a_resendCount = 0;
    }

    l_controllerDriver->a_api.a_send(
        p_device->a_parent,
        *l_deviceAddress,
        p_value
    );
}

static uint8_t ps2PortReadBuffer(
    struct ts_device *p_device
) {
    volatile struct ts_deviceDriverDataPs2Port *l_driverData =
        (volatile struct ts_deviceDriverDataPs2Port *)p_device->a_driverData;

    while(l_driverData->a_receiveBufferLength == 0) {
        archHalt();
    }

    uint8_t l_returnValue = l_driverData->a_receiveBuffer[l_driverData->a_receiveBufferReadIndex++];

    if(l_driverData->a_receiveBufferReadIndex == C_PS2_RECEIVE_BUFFER_SIZE) {
        l_driverData->a_receiveBufferReadIndex = 0;
    }

    l_driverData->a_receiveBufferLength--;

    return l_returnValue;
}
