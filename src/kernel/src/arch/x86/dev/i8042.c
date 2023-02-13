#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "arch/x86/isr.h"
#include "dev/ps2.h"
#include "dev/timer.h"
#include "common.h"
#include "debug.h"

#define C_IOPORT_8042_DATA 0x60
#define C_IOPORT_8042_COMMAND 0x64
#define C_IOPORT_8042_STATUS 0x64

#define C_8042_CMD_READ_RAM 0x20
#define C_8042_CMD_WRITE_RAM 0x60
#define C_8042_CMD_PORT_2_DISABLE 0xa7
#define C_8042_CMD_PORT_2_ENABLE 0xa8
#define C_8042_CMD_PORT_2_TEST 0xa9
#define C_8042_CMD_CONTROLLER_TEST 0xaa
#define C_8042_CMD_PORT_1_TEST 0xab
#define C_8042_CMD_READ_RAM_ALL 0xac
#define C_8042_CMD_PORT_1_DISABLE 0xad
#define C_8042_CMD_PORT_1_ENABLE 0xae
#define C_8042_CMD_CONTROLLER_READ 0xc0
#define C_8042_CMD_INPUT_STATUS_0_3 0xc1
#define C_8042_CMD_INPUT_STATUS_4_7 0xc2
#define C_8042_CMD_CONTROLLER_OUTPUT_PORT_READ 0xd0
#define C_8042_CMD_CONTROLLER_OUTPUT_PORT_WRITE 0xd1
#define C_8042_CMD_PORT_1_FAKE_READ 0xd2
#define C_8042_CMD_PORT_2_FAKE_READ 0xd3
#define C_8042_CMD_PORT_2_WRITE 0xd4
#define C_8042_CMD_PULSE 0xf0
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

struct ts_deviceDataPs2Port {
    uint8_t a_receiveBuffer[C_PS2_RECEIVE_BUFFER_SIZE];
    int a_receiveBufferReadIndex;
    int a_receiveBufferWriteIndex;
    int a_receiveBufferLength;
    bool a_working;
    bool a_receivedSelfTest;
    bool a_receivedEcho;
    bool a_receivedAck;
    uint8_t a_lastSentByte;
    int a_resendCount;
    struct ts_device a_children[2];
};

struct ts_deviceDataPs2 {
    struct ts_deviceDataPs2Port a_ports[2];
};

static int i8042Init(struct ts_device *p_device);
static bool i8042CanReceive(struct ts_device *p_device, int p_port);
static uint8_t i8042Receive(struct ts_device *p_device, int p_port);
static void i8042Send(struct ts_device *p_device, int p_port, uint8_t p_value);
static void i8042WaitAck(struct ts_device *p_device, int p_port);
static void i8042WaitSelfTest(struct ts_device *p_device, int p_port);
static void i8042WaitRead(void);
static void i8042WaitWrite(void);
static void i8042InitPort(struct ts_device *p_device, int p_port);
static void i8042InterruptHandler(
    struct ts_isrRegisters *p_registers,
    struct ts_device *p_device
);
static void i8042FlushReceiveBuffer(struct ts_device *p_device, int p_port);

const struct ts_deviceDriverPs2 g_devDriverI8042 = {
    .a_driver = {
        .a_name = "8042 PS/2 controller",
        .a_init = i8042Init
    },
    .a_canReceive = i8042CanReceive,
    .a_receive = i8042Receive,
    .a_send = i8042Send
};

static struct ts_deviceDataPs2 s_deviceDataI8042;

static int i8042Init(struct ts_device *p_device) {
    p_device->a_driverData = &s_deviceDataI8042;

    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;

    debugPrint("i8042: init()\n");

    // By default, we assume that the controller has two ports, and that both
    // of them are working (a device is plugged and works correctly). We will
    // test these assumptions later.
    l_deviceData->a_ports[0].a_working = true;
    l_deviceData->a_ports[1].a_working = true;

    // Disable both PS/2 ports
    debugPrint("i8042: Disabling ports...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_1_DISABLE);
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_DISABLE);

    // Flush output buffer
    inb(C_IOPORT_8042_DATA);

    // Configure controller
    debugPrint("i8042: Configuring controller...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_READ_RAM);
    i8042WaitRead();

    uint8_t l_configurationByte = inb(C_IOPORT_8042_DATA);

    l_configurationByte &= ~((1 << 0) | (1 << 1) | (1 << 6));

    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_WRITE_RAM);
    i8042WaitWrite();
    outb(C_IOPORT_8042_DATA, l_configurationByte);

    // If the 5th bit of the controller configuration byte was clear, we know
    // that there is no second port (it should be set because of
    // C_8042_CMD_PORT_2_DISABLE).
    if((l_configurationByte & (1 << 5)) == 0) {
        debugPrint("i8042: Second port is not present.\n");
        l_deviceData->a_ports[1].a_working = false;
    }

    // Perform controller self-test
    debugPrint("i8042: Performing controller self-test...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_CONTROLLER_TEST);
    i8042WaitRead();

    uint8_t l_result = inb(C_IOPORT_8042_DATA);

    if(l_result == C_PS2_RESPONSE_SELF_TEST_FAILED_1) {
        debugPrint("i8042: Controller self-test failed.\n");
        return 1;
    } else if(l_result != 0x55) {
        debugPrint("i8042: Controller self-test failed (unknown answer).\n");
        return 1;
    }

    debugPrint("i8042: Controller self-test passed.\n");

    // Detect second port
    if(l_deviceData->a_ports[1].a_working) {
        debugPrint("i8042: Detecting second port...\n");
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_ENABLE);
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_READ_RAM);
        i8042WaitRead();
        l_configurationByte = inb(C_IOPORT_8042_DATA);

        // If the 5th bit of the controller configuration byte was set, we know
        // that there is no second port (it should be clear because of
        // C_8042_CMD_PORT_2_ENABLE).
        if((l_configurationByte & (1 << 5)) != 0) {
            debugPrint("i8042: Second port is not present.\n");
            l_deviceData->a_ports[1].a_working = false;
        } else {
            debugPrint("i8042: Second port was found.\n");
            outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_DISABLE);
        }
    }

    // Test the first PS/2 port
    debugPrint("i8042: Testing first port...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_1_TEST);
    i8042WaitRead();
    l_result = inb(C_IOPORT_8042_DATA);

    if(l_result != 0) {
        debugPrint("i8042: First port is not working.\n");
        l_deviceData->a_ports[0].a_working = false;
    }

    // Test the second PS/2 port
    if(l_deviceData->a_ports[1].a_working) {
        debugPrint("i8042: Testing second port...\n");
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_TEST);
        i8042WaitRead();
        l_result = inb(C_IOPORT_8042_DATA);

        if(l_result != 0) {
            debugPrint("i8042: Second port is not working.\n");
            l_deviceData->a_ports[1].a_working = false;
        }
    }

    // Enable all working ports and their interrupts
    if(l_deviceData->a_ports[0].a_working) {
        debugPrint("i8042: Enabling first port...\n");

        i8042FlushReceiveBuffer(p_device, 0);

        isrSetHandler(33, (tf_isrHandler *)i8042InterruptHandler, p_device);

        l_configurationByte |= 1 << 0;
        l_configurationByte &= ~(1 << 4);
    }

    if(l_deviceData->a_ports[1].a_working) {
        debugPrint("i8042: Enabling second port...\n");

        i8042FlushReceiveBuffer(p_device, 1);

        isrSetHandler(44, (tf_isrHandler *)i8042InterruptHandler, p_device);

        l_configurationByte |= 1 << 1;
        l_configurationByte &= ~(1 << 5);
    }

    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_WRITE_RAM);
    i8042WaitWrite();
    outb(C_IOPORT_8042_DATA, l_configurationByte);

    // Initialize devices
    for(int l_port = 0; l_port < 2; l_port++) {
        if(l_deviceData->a_ports[l_port].a_working) {
            i8042InitPort(p_device, l_port);
        }
    }

    debugPrint("i8042: PS/2 controller initialized.\n");

    return 0;
}

static bool i8042CanReceive(struct ts_device *p_device, int p_port) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    return l_port->a_receiveBufferLength != 0;
}

static uint8_t i8042Receive(struct ts_device *p_device, int p_port) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    while(!i8042CanReceive(p_device, p_port)) {
        hlt();
    }

    cli();

    uint8_t l_returnValue = l_port->a_receiveBuffer[
        l_port->a_receiveBufferReadIndex++
    ];

    if(l_port->a_receiveBufferReadIndex == C_PS2_RECEIVE_BUFFER_SIZE) {
        l_port->a_receiveBufferReadIndex = 0;
    }

    l_port->a_receiveBufferLength--;

    sti();

    return l_returnValue;
}

static void i8042Send(struct ts_device *p_device, int p_port, uint8_t p_value) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    l_port->a_receivedAck = false;

    if(p_port == 1) {
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_WRITE);
    }

    i8042WaitWrite();

    if(p_value != C_PS2_CMD_RESEND) {
        l_port->a_lastSentByte = p_value;
        l_port->a_resendCount = 0;
    }

    outb(C_IOPORT_8042_DATA, p_value);

    if(p_value != C_PS2_CMD_ECHO) {
        i8042WaitAck(p_device, p_port);
    }
}

static void i8042WaitAck(struct ts_device *p_device, int p_port) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    uint64_t l_startTime = timerGetTime();
    uint64_t l_endTime = l_startTime + C_PS2_TIMEOUT_RESET;

    while(
        (!l_port->a_receivedAck)
        && (timerGetTime() < l_endTime)
    ) {
        hlt();
    }

    if(!l_port->a_receivedAck) {
        l_port->a_working = false;
        debugPrint("i8042: Device ACK timed out.\n");
    }
}

static void i8042WaitSelfTest(struct ts_device *p_device, int p_port) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    uint64_t l_startTime = timerGetTime();
    uint64_t l_endTime = l_startTime + C_PS2_TIMEOUT_RESET;

    while(
        (!l_port->a_receivedSelfTest)
            && (timerGetTime() < l_endTime)
    ) {
        hlt();
    }

    if(!l_port->a_receivedSelfTest) {
        l_port->a_working = false;
        debugPrint("i8042: Device self-test timed out.\n");
    }
}

static void i8042WaitRead(void) {
    uint8_t l_status;

    do {
        l_status = inb(C_IOPORT_8042_STATUS);
    } while((l_status & (1 << 0)) == 0);
}

static void i8042WaitWrite(void) {
    uint8_t l_status;

    do {
        l_status = inb(C_IOPORT_8042_STATUS);
    } while((l_status & (1 << 1)) != 0);
}

static void i8042InitPort(struct ts_device *p_device, int p_port) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    debugPrint("i8042: Initializing device on ");
    debugPrint(p_port == 0 ? "first" : "second");
    debugPrint(" port.\n");

    l_port->a_receivedSelfTest = false;
    l_port->a_receivedEcho = false;
    l_port->a_receivedAck = false;

    // Send reset command
    i8042Send(p_device, p_port, C_PS2_CMD_RESET);
    i8042WaitSelfTest(p_device, p_port);

    if(!l_port->a_working) {
        return;
    }

    // Disable scanning
    i8042Send(p_device, p_port, C_PS2_CMD_SCAN_DISABLE);

    if(!l_port->a_working) {
        return;
    }

    // Flush buffer
    i8042FlushReceiveBuffer(p_device, p_port);

    // Send identify command
    i8042Send(p_device, p_port, C_PS2_CMD_IDENTIFY);

    if(!l_port->a_working) {
        return;
    }

    uint8_t l_identification[2];
    int l_identificationLength = 0;
    uint64_t l_timeout = timerGetTime() + C_PS2_TIMEOUT_IDENTIFY;

    while(l_identificationLength < 2) {
        while(
            (timerGetTime() < l_timeout)
                && (!i8042CanReceive(p_device, p_port))
        ) {
            hlt();
        }

        if(!i8042CanReceive(p_device, p_port)) {
            break;
        }

        l_identification[l_identificationLength++] = i8042Receive(
            p_device,
            p_port
        );
    }

    debugPrint("i8042: Identification data: ");

    for(int l_index = 0; l_index < l_identificationLength; l_index++) {
        debugPrintHex8(l_identification[l_index]);
        debugPrint(" ");
    }

    debugPrint("\n");
}

static void i8042InterruptHandler(
    struct ts_isrRegisters *p_registers,
    struct ts_device *p_device
) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;

    int l_portNumber;

    if(p_registers->a_interruptNumber == 33) { // IRQ1
        l_portNumber = 0;
    } else if(p_registers->a_interruptNumber == 44) { // IRQ2
        l_portNumber = 1;
    } else {
        return;
    }

    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[l_portNumber];

    uint8_t l_value = inb(C_IOPORT_8042_DATA);

    if(l_value == C_PS2_RESPONSE_SELF_TEST_PASSED) {
        l_port->a_receivedSelfTest = true;
        l_port->a_working = true;
    } else if(l_value == C_PS2_RESPONSE_ECHO) {
        l_port->a_receivedEcho = true;
    } else if(l_value == C_PS2_RESPONSE_ACK) {
        l_port->a_receivedAck = true;
    } else if(
        (l_value == C_PS2_RESPONSE_SELF_TEST_FAILED_1)
        || (l_value == C_PS2_RESPONSE_SELF_TEST_FAILED_2)
    ) {
        l_port->a_receivedSelfTest = true;
        l_port->a_working = false;
    } else if(l_value == C_PS2_RESPONSE_RESEND) {
        l_port->a_resendCount++;

        if(l_port->a_resendCount == C_PS2_MAX_RESEND_COUNT) {
            debugPrint("i8042: Too many resend requests.\n");
            l_port->a_working = false;
            return;
        }

        i8042Send(p_device, l_portNumber, l_port->a_lastSentByte);
    } else {
        if(l_port->a_receiveBufferLength == C_PS2_RECEIVE_BUFFER_SIZE) {
            return;
        }

        l_port->a_receiveBuffer[l_port->a_receiveBufferWriteIndex++] = l_value;
        l_port->a_receiveBufferLength++;

        if(l_port->a_receiveBufferWriteIndex == C_PS2_RECEIVE_BUFFER_SIZE) {
            l_port->a_receiveBufferWriteIndex = 0;
        }
    }
}

static void i8042FlushReceiveBuffer(struct ts_device *p_device, int p_port) {
    volatile struct ts_deviceDataPs2 *l_deviceData =
        (struct ts_deviceDataPs2 *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2Port *l_port =
        (struct ts_deviceDataPs2Port *)&l_deviceData->a_ports[p_port];

    l_port->a_receiveBufferLength = 0;
    l_port->a_receiveBufferReadIndex = 0;
    l_port->a_receiveBufferWriteIndex = 0;
}
