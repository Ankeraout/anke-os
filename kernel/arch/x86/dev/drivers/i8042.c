#include <stdbool.h>
#include <stdint.h>

#include <kernel/arch/x86/inline.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/dev/drivers/ps2port.h>
#include <kernel/dev/ps2.h>
#include <kernel/dev/timer.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/klibc/string.h>
#include <kernel/common.h>
#include <kernel/debug.h>

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
#define C_PS2_RESPONSE_SELF_TEST_FAILED 0xfc

struct ts_deviceDataPs2ControllerPort {
    bool a_working;
    struct ts_device a_device;
};

struct ts_deviceDataPs2Controller {
    struct ts_deviceDataPs2ControllerPort a_ports[2];
};

static int i8042Init(struct ts_device *p_device);
static void i8042WaitRead(void);
static void i8042WaitWrite(void);
static void i8042InitPort(struct ts_device *p_device, int p_port);
static void i8042InterruptHandler(
    struct ts_isrRegisters *p_registers,
    struct ts_device *p_device
);
static size_t i8042DriverApiGetChildCount(struct ts_device *p_device);
static struct ts_device *i8042DriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static void i8042DriverApiSend(
    struct ts_device *p_device,
    enum te_deviceAddressPs2 p_port,
    uint8_t p_value
);

const struct ts_deviceDriverPs2Controller g_deviceDriverI8042 = {
    .a_base = {
        .a_name = "Intel 8042 PS/2 controller",
        .a_api = {
            .a_init = i8042Init,
            .a_getChild = i8042DriverApiGetChild,
            .a_getChildCount = i8042DriverApiGetChildCount,
            .a_isSupported = NULL,
        }
    },
    .a_api = {
        .a_send = i8042DriverApiSend
    }
};

static int i8042Init(struct ts_device *p_device) {
    p_device->a_driverData = kmalloc(sizeof(struct ts_deviceDataPs2Controller));

    if(p_device->a_driverData == NULL) {
        debug("i8042: Failed to allocate memory for driver data.\n");
        return 1;
    }

    memset(p_device->a_driverData, 0, sizeof(struct ts_deviceDataPs2Controller));

    volatile struct ts_deviceDataPs2Controller *l_deviceData =
        (struct ts_deviceDataPs2Controller *)p_device->a_driverData;

    // By default, we assume that the controller has two ports, and that both
    // of them are working (a device is plugged and works correctly). We will
    // test these assumptions later.
    l_deviceData->a_ports[0].a_working = true;
    l_deviceData->a_ports[1].a_working = true;

    // Disable both PS/2 ports
    debug("i8042: Disabling ports...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_1_DISABLE);
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_DISABLE);

    // Flush output buffer
    inb(C_IOPORT_8042_DATA);

    // Configure controller
    debug("i8042: Configuring controller...\n");
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
        debug("i8042: Second port is not present.\n");
        l_deviceData->a_ports[1].a_working = false;
    }

    // Perform controller self-test
    debug("i8042: Performing controller self-test...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_CONTROLLER_TEST);
    i8042WaitRead();

    uint8_t l_result = inb(C_IOPORT_8042_DATA);

    if(l_result == C_PS2_RESPONSE_SELF_TEST_FAILED) {
        debug("i8042: Controller self-test failed.\n");
        return 1;
    } else if(l_result != 0x55) {
        debug("i8042: Controller self-test failed (unknown answer).\n");
        return 1;
    }

    debug("i8042: Controller self-test passed.\n");

    // Detect second port
    if(l_deviceData->a_ports[1].a_working) {
        debug("i8042: Detecting second port...\n");
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_ENABLE);
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_READ_RAM);
        i8042WaitRead();
        l_configurationByte = inb(C_IOPORT_8042_DATA);

        // If the 5th bit of the controller configuration byte was set, we know
        // that there is no second port (it should be clear because of
        // C_8042_CMD_PORT_2_ENABLE).
        if((l_configurationByte & (1 << 5)) != 0) {
            debug("i8042: Second port is not present.\n");
            l_deviceData->a_ports[1].a_working = false;
        } else {
            debug("i8042: Second port was found.\n");
            outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_DISABLE);
        }
    }

    // Test the first PS/2 port
    debug("i8042: Testing first port...\n");
    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_1_TEST);
    i8042WaitRead();
    l_result = inb(C_IOPORT_8042_DATA);

    if(l_result != 0) {
        debug("i8042: First port is not working.\n");
        l_deviceData->a_ports[0].a_working = false;
    }

    // Test the second PS/2 port
    if(l_deviceData->a_ports[1].a_working) {
        debug("i8042: Testing second port...\n");
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_TEST);
        i8042WaitRead();
        l_result = inb(C_IOPORT_8042_DATA);

        if(l_result != 0) {
            debug("i8042: Second port is not working.\n");
            l_deviceData->a_ports[1].a_working = false;
        }
    }

    // Enable all working ports and their interrupts
    if(l_deviceData->a_ports[0].a_working) {
        debug("i8042: Enabling first port...\n");

        i8042InitPort(p_device, 0);

        isrSetHandler(33, (tf_isrHandler *)i8042InterruptHandler, p_device);

        l_configurationByte |= 1 << 0;
        l_configurationByte &= ~(1 << 4);
    }

    if(l_deviceData->a_ports[1].a_working) {
        debug("i8042: Enabling second port...\n");

        i8042InitPort(p_device, 1);

        isrSetHandler(44, (tf_isrHandler *)i8042InterruptHandler, p_device);

        l_configurationByte |= 1 << 1;
        l_configurationByte &= ~(1 << 5);
    }

    outb(C_IOPORT_8042_COMMAND, C_8042_CMD_WRITE_RAM);
    i8042WaitWrite();
    outb(C_IOPORT_8042_DATA, l_configurationByte);

    debug("i8042: Initializing ports...\n");

    // Detect devices
    for(int l_port = 0; l_port < 2; l_port++) {
        if(l_deviceData->a_ports[l_port].a_working) {
            const struct ts_deviceDriverPs2Port *l_portDriver =
                (const struct ts_deviceDriverPs2Port *)l_deviceData->a_ports[l_port].a_device.a_driver;

            l_portDriver->a_api.a_detect(
                (struct ts_device *)&l_deviceData->a_ports[l_port].a_device
            );
        }
    }

    debug("i8042: PS/2 controller initialized.\n");

    return 0;
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
    volatile struct ts_deviceDataPs2Controller *l_deviceData =
        (struct ts_deviceDataPs2Controller *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2ControllerPort *l_port =
        (struct ts_deviceDataPs2ControllerPort *)&l_deviceData->a_ports[p_port];

    struct ts_device *l_portDevice =
        (struct ts_device *)&l_deviceData->a_ports[p_port].a_device;

    *((enum te_deviceAddressPs2 *)l_portDevice->a_address.a_address) = p_port;
    l_portDevice->a_driver =
        (const struct ts_deviceDriver *)&g_deviceDriverPs2Port;
    l_portDevice->a_parent = p_device;

    if(g_deviceDriverPs2Port.a_base.a_api.a_init(l_portDevice) != 0) {
        debug("i8042: Failed to initialize port driver.\n");
        kfree(l_portDevice);
        l_portDevice = NULL;
        l_port->a_working = false;
    }
}

static void i8042InterruptHandler(
    struct ts_isrRegisters *p_registers,
    struct ts_device *p_device
) {
    volatile struct ts_deviceDataPs2Controller *l_deviceData =
        (struct ts_deviceDataPs2Controller *)p_device->a_driverData;

    int l_portNumber;

    if(p_registers->a_interruptNumber == 33) { // IRQ1
        l_portNumber = 0;
    } else if(p_registers->a_interruptNumber == 44) { // IRQ2
        l_portNumber = 1;
    } else {
        return;
    }

    volatile struct ts_deviceDataPs2ControllerPort *l_port =
        (struct ts_deviceDataPs2ControllerPort *)&l_deviceData->a_ports[l_portNumber];
    const struct ts_deviceDriverPs2Port *l_portDriver =
        (const struct ts_deviceDriverPs2Port *)l_port->a_device.a_driver;
    struct ts_device *l_portDevice = (struct ts_device *)&l_port->a_device;

    i8042WaitRead();
    uint8_t l_data = inb(C_IOPORT_8042_DATA);

    if(l_port->a_working) {
        l_portDriver->a_api.a_receive(l_portDevice, l_data);
    }
}

static size_t i8042DriverApiGetChildCount(struct ts_device *p_device) {
    volatile struct ts_deviceDataPs2Controller *l_deviceData =
        (struct ts_deviceDataPs2Controller *)p_device->a_driverData;

    size_t l_childCount = 0;

    for(size_t l_index = 0; l_index < 2; l_index++) {
        if(l_deviceData->a_ports[l_index].a_working) {
            l_childCount++;
        }
    }

    return l_childCount;
}

static struct ts_device *i8042DriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    size_t l_workingDeviceCount = i8042DriverApiGetChildCount(p_device);

    if(p_index >= l_workingDeviceCount) {
        return NULL;
    }

    struct ts_deviceDataPs2Controller *l_deviceData =
        (struct ts_deviceDataPs2Controller *)p_device->a_driverData;

    if(!l_deviceData->a_ports[0].a_working) {
        p_index--;
    }

    return &l_deviceData->a_ports[p_index].a_device;
}

static void i8042DriverApiSend(
    struct ts_device *p_device,
    enum te_deviceAddressPs2 p_port,
    uint8_t p_value
) {
    volatile struct ts_deviceDataPs2Controller *l_deviceData =
        (struct ts_deviceDataPs2Controller *)p_device->a_driverData;
    volatile struct ts_deviceDataPs2ControllerPort *l_port =
        (struct ts_deviceDataPs2ControllerPort *)&l_deviceData->a_ports[p_port];

    if(!l_port->a_working) {
        return;
    }

    const enum te_deviceAddressPs2 *l_deviceAddress =
        (const enum te_deviceAddressPs2 *)l_port->a_device.a_address.a_address;

    if(*l_deviceAddress == E_DEVICEADDRESS_PS2_1) {
        outb(C_IOPORT_8042_COMMAND, C_8042_CMD_PORT_2_WRITE);
    }

    i8042WaitWrite();
    outb(C_IOPORT_8042_DATA, p_value);
}
