#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "arch/x86/isr.h"
#include "arch/x86/pit.h"
#include "debug.h"

#define C_IOPORT_PS2_DATA 0x60
#define C_IOPORT_PS2_COMMAND 0x64
#define C_IOPORT_PS2_STATUS 0x64

#define C_PS2_CMD_READ_RAM 0x20
#define C_PS2_CMD_WRITE_RAM 0x60
#define C_PS2_CMD_PORT_2_DISABLE 0xa7
#define C_PS2_CMD_PORT_2_ENABLE 0xa8
#define C_PS2_CMD_PORT_2_TEST 0xa9
#define C_PS2_CMD_CONTROLLER_TEST 0xaa
#define C_PS2_CMD_PORT_1_TEST 0xab
#define C_PS2_CMD_READ_RAM_ALL 0xac
#define C_PS2_CMD_PORT_1_DISABLE 0xad
#define C_PS2_CMD_PORT_1_ENABLE 0xae
#define C_PS2_CMD_CONTROLLER_READ 0xc0
#define C_PS2_CMD_INPUT_STATUS_0_3 0xc1
#define C_PS2_CMD_INPUT_STATUS_4_7 0xc2
#define C_PS2_CMD_CONTROLLER_OUTPUT_PORT_READ 0xd0
#define C_PS2_CMD_CONTROLLER_OUTPUT_PORT_WRITE 0xd1
#define C_PS2_CMD_PORT_1_FAKE_READ 0xd2
#define C_PS2_CMD_PORT_2_FAKE_READ 0xd3
#define C_PS2_CMD_PORT_2_WRITE 0xd4
#define C_PS2_CMD_PULSE 0xf0
#define C_PS2_RECEIVE_BUFFER_SIZE 16

#define C_PS2_TIMEOUT 100

enum te_ps2DeviceType {
    E_PS2_DEVICETYPE_UNKNOWN,
    E_PS2_DEVICETYPE_KEYBOARD_AT,
    E_PS2_DEVICETYPE_KEYBOARD_MF2,
    E_PS2_DEVICETYPE_MOUSE,
    E_PS2_DEVICETYPE_MOUSE_WHEEL,
    E_PS2_DEVICETYPE_MOUSE_5BUTTON
};

struct ts_ps2Port {
    bool a_working;
    int a_receiveBufferLength;
    int a_receiveBufferWriteIndex;
    int a_receiveBufferReadIndex;
    uint8_t a_receiveBuffer[C_PS2_RECEIVE_BUFFER_SIZE];
    enum te_ps2DeviceType a_deviceType;
    bool a_pendingSelfTest;
    bool a_flagEcho;
    bool a_flagAck;
    uint8_t a_lastSentByte;
    int a_retryCount;
};

static const char *s_ps2DeviceName[] = {
    "Unknown",
    "AT keyboard",
    "MF2 keyboard",
    "Standard mouse",
    "Standard mouse with scrollwheel",
    "Standard mouse with 5 buttons"
};

static volatile struct ts_ps2Port s_ps2Ports[2];

static void ps2WaitAck(int p_port);
static void ps2WaitSelfTest(int p_port);
static void ps2WaitRead(void);
static void ps2WaitWrite(void);
static void ps2InitDevice(int p_port);
static void ps2InterruptHandler(struct ts_isrRegisters *p_registers);
static void ps2FlushReceiveBuffer(int p_port);

void ps2Init(void) {
    debugPrint("ps2: Initializing 8042 PS/2 controller.\n");

    // By default we consider that the controller has 2 ports, we will test them
    // later to check if they are present and working.
    s_ps2Ports[0].a_working = true;
    s_ps2Ports[1].a_working = true;

    // Disable ports
    debugPrint("ps2: Disabling ports.\n");
    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_1_DISABLE);
    iowait();
    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_2_DISABLE);
    iowait();

    // Flush output buffer
    debugPrint("ps2: Flushing output buffer.\n");
    inb(C_IOPORT_PS2_DATA);

    // Configure controller
    debugPrint("ps2: Configuring controller.\n");
    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_READ_RAM);
    iowait();
    ps2WaitRead();
    uint8_t l_configurationByte = inb(C_IOPORT_PS2_DATA);

    l_configurationByte &= ~((1 << 0) | (1 << 1) | (1 << 6));

    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_WRITE_RAM);
    iowait();
    ps2WaitWrite();
    outb(C_IOPORT_PS2_DATA, l_configurationByte);
    iowait();

    // If bit 5 is clear, we know that there is no 2nd port (it should be set
    // because of command C_PS2_CMD_PORT_2_DISABLE).
    if((l_configurationByte & (1 << 5)) == 0) {
        debugPrint("ps2: Second port is not present.\n");
        s_ps2Ports[1].a_working = false;
    }

    // Perform PS/2 controller self-test
    debugPrint("ps2: Performing controller self-test.\n");
    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_CONTROLLER_TEST);
    iowait();
    ps2WaitRead();
    uint8_t l_result = inb(C_IOPORT_PS2_DATA);

    if(l_result == 0xfc) {
        debugPrint("ps2: PS/2 controller self-test failed.\n");
        return;
    } else if(l_result != 0x55) {
        debugPrint("ps2: PS/2 controller self-test failed (unknown answer).\n");
        return;
    }

    debugPrint("ps2: PS/2 controller self-test passed.\n");

    // Detect second port
    if(s_ps2Ports[1].a_working) {
        debugPrint("ps2: Detecting second port...\n");
        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_2_ENABLE);
        iowait();

        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_READ_RAM);
        iowait();
        ps2WaitRead();
        l_configurationByte = inb(C_IOPORT_PS2_DATA);

        // If bit 5 is not cleared, we know that there is no 2nd port (it should be
        // cleared because of command C_PS2_CMD_PORT_2_ENABLE).
        if((l_configurationByte & (1 << 5)) != 0) {
            debugPrint("ps2: Second port is not present.\n");
            s_ps2Ports[1].a_working = false;
        }

        // Disable the second PS/2 port again
        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_2_DISABLE);
        iowait();
    }

    // Test the first PS/2 port
    debugPrint("ps2: Testing first port.\n");
    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_1_TEST);
    iowait();
    ps2WaitRead();
    l_result = inb(C_IOPORT_PS2_DATA);

    if(l_result != 0) {
        debugPrint("ps2: First port is not working.\n");
        s_ps2Ports[0].a_working = false;
    }

    // Test the second PS/2 port
    if(s_ps2Ports[1].a_working) {
        debugPrint("ps2: Testing second port...\n");
        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_2_TEST);
        iowait();
        ps2WaitRead();
        l_result = inb(C_IOPORT_PS2_DATA);

        if(l_result != 0) {
            debugPrint("ps2: Second port is not working.\n");
            s_ps2Ports[1].a_working = false;
        }
    }

    // Enable all working ports and their interrupts
    if(s_ps2Ports[0].a_working) {
        debugPrint("ps2: Enabling first port.\n");

        s_ps2Ports[0].a_receiveBufferLength = 0;
        s_ps2Ports[0].a_receiveBufferReadIndex = 0;
        s_ps2Ports[0].a_receiveBufferWriteIndex = 0;

        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_1_ENABLE);
        iowait();
        l_configurationByte |= 1 << 0;

        isrSetHandler(33, ps2InterruptHandler);
    }

    if(s_ps2Ports[1].a_working) {
        debugPrint("ps2: Enabling second port.\n");

        s_ps2Ports[1].a_receiveBufferLength = 0;
        s_ps2Ports[1].a_receiveBufferReadIndex = 0;
        s_ps2Ports[1].a_receiveBufferWriteIndex = 0;

        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_2_ENABLE);
        iowait();
        l_configurationByte |= 1 << 1;

        isrSetHandler(44, ps2InterruptHandler);
    }

    outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_WRITE_RAM);
    iowait();
    ps2WaitWrite();
    outb(C_IOPORT_PS2_DATA, l_configurationByte);

    // Initialize devices
    debugPrint("ps2: Initializing devices.\n");

    for(int l_port = 0; l_port < 2; l_port++) {
        if(s_ps2Ports[l_port].a_working) {
            ps2InitDevice(l_port);
        }
    }
}

bool ps2CanRead(int p_port) {
    return s_ps2Ports[p_port].a_receiveBufferLength != 0;
}

uint8_t ps2Read(int p_port) {
    while(s_ps2Ports[p_port].a_receiveBufferLength == 0);

    cli();

    uint8_t l_returnValue = s_ps2Ports[p_port].a_receiveBuffer[s_ps2Ports[p_port].a_receiveBufferReadIndex++];

    if(s_ps2Ports[p_port].a_receiveBufferReadIndex == C_PS2_RECEIVE_BUFFER_SIZE) {
        s_ps2Ports[p_port].a_receiveBufferReadIndex = 0;
    }

    s_ps2Ports[p_port].a_receiveBufferLength--;

    sti();

    return l_returnValue;
}

void ps2Write(int p_port, uint8_t p_value) {
    s_ps2Ports[p_port].a_flagAck = false;

    if(p_port == 1) {
        outb(C_IOPORT_PS2_COMMAND, C_PS2_CMD_PORT_2_WRITE);
        iowait();
    }

    ps2WaitWrite();
    outb(C_IOPORT_PS2_DATA, p_value);
    iowait();

    if(p_value != 0xfe) {
        s_ps2Ports[p_port].a_lastSentByte = p_value;
        s_ps2Ports[p_port].a_retryCount = 0;
    }

    if(p_value != 0xee) {
        ps2WaitAck(p_port);
    }
}

static void ps2WaitAck(int p_port) {
    uint64_t l_startTime = pitGetTimeMilliseconds();
    uint64_t l_endTime = l_startTime + C_PS2_TIMEOUT;

    while((!s_ps2Ports[p_port].a_flagAck) && (pitGetTimeMilliseconds() < l_endTime)) {
        hlt();
    }

    if(!s_ps2Ports[p_port].a_flagAck) {
        s_ps2Ports[p_port].a_working = false;
        debugPrint("ps2: Device ACK timed out.\n");
    }
}

static void ps2WaitSelfTest(int p_port) {
    uint64_t l_startTime = pitGetTimeMilliseconds();
    uint64_t l_endTime = l_startTime + C_PS2_TIMEOUT;

    while((s_ps2Ports[p_port].a_pendingSelfTest) && (pitGetTimeMilliseconds() < l_endTime)) {
        hlt();
    }

    if(s_ps2Ports[p_port].a_pendingSelfTest) {
        s_ps2Ports[p_port].a_working = false;
        debugPrint("ps2: Device self-test timed out.\n");
    }
}

static void ps2InitDevice(int p_port) {
    debugPrint("ps2: Initializing device on ");
    debugPrint(p_port == 0 ? "first" : "second");
    debugPrint(" port.\n");

    s_ps2Ports[p_port].a_pendingSelfTest = false;
    s_ps2Ports[p_port].a_flagEcho = false;
    s_ps2Ports[p_port].a_flagAck = false;

    // TODO: read time-out

    // Send reset command
    ps2Write(p_port, 0xff);
    ps2WaitSelfTest(p_port);

    if(!s_ps2Ports[p_port].a_working) {
        return;
    }

    // Disable scanning
    ps2Write(p_port, 0xf5);

    if(!s_ps2Ports[p_port].a_working) {
        return;
    }

    // Flush buffer
    ps2FlushReceiveBuffer(p_port);

    // Send identify command
    ps2Write(p_port, 0xf2);

    if(!s_ps2Ports[p_port].a_working) {
        return;
    }

    uint8_t l_buffer[2];
    int l_bufferLength = 0;

    while(l_bufferLength < 2) {
        pitSleep(C_PS2_TIMEOUT);

        if(!ps2CanRead(p_port)) {
            break;
        }

        l_buffer[l_bufferLength++] = ps2Read(p_port);
    }

    s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_UNKNOWN;

    if(l_bufferLength == 0) {
        s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_KEYBOARD_AT;
    } else if(l_bufferLength == 1) {
        if(l_buffer[0] == 0x00) {
            s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_MOUSE;
        } else if(l_buffer[1] == 0x03) {
            s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_MOUSE_WHEEL;
        } else if(l_buffer[1] == 0x04) {
            s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_MOUSE_5BUTTON;
        }
    } else if(l_bufferLength == 2) {
        if(l_buffer[0] == 0xab) {
            if((l_buffer[1] == 0x83) || (l_buffer[1] == 0xc1)) {
                s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_KEYBOARD_MF2;
            } else {
                s_ps2Ports[p_port].a_deviceType = E_PS2_DEVICETYPE_UNKNOWN;
            }
        }
    }

    debugPrint("ps2: Identified device: ");
    debugPrint(s_ps2DeviceName[s_ps2Ports[p_port].a_deviceType]);
    debugPrint(".\n");
    debugPrint("ps2: Identification data: ");

    for(int l_index = 0; l_index < l_bufferLength; l_index++) {
        debugPrintHex8(l_buffer[l_index]);
        debugPrint(" ");
    }

    debugPrint("\n");
}

static void ps2WaitRead(void) {
    uint8_t l_status;

    do {
        l_status = inb(C_IOPORT_PS2_STATUS);
    } while((l_status & (1 << 0)) == 0);
}

static void ps2WaitWrite(void) {
    uint8_t l_status;

    do {
        l_status = inb(C_IOPORT_PS2_STATUS);
    } while((l_status & (1 << 1)) != 0);
}

static void ps2InterruptHandler(struct ts_isrRegisters *p_registers) {
    debugPrint("ps2: Interrupt\n");

    int l_port;

    if(p_registers->a_interruptNumber == 33) { // IRQ1
        l_port = 0;
    } else if(p_registers->a_interruptNumber == 44) { // IRQ12
        l_port = 1;
    }

    uint8_t l_value = inb(C_IOPORT_PS2_DATA);

    if(l_value == 0xaa) {
        s_ps2Ports[l_port].a_pendingSelfTest = false; // Self-test passed
        s_ps2Ports[l_port].a_working = true;
    } else if(l_value == 0xee) {
        s_ps2Ports[l_port].a_flagEcho = true;
    } else if(l_value == 0xfa) {
        s_ps2Ports[l_port].a_flagAck = true;
    } else if((l_value == 0xfc) || (l_value == 0xfd)) {
        s_ps2Ports[l_port].a_working = false;
    } else if(l_value == 0xfe) {
        s_ps2Ports[l_port].a_retryCount++;

        if(s_ps2Ports[l_port].a_retryCount == 3) {
            debugPrint("ps2: Too many resend requests.\n");
            s_ps2Ports[l_port].a_working = false;
            return;
        }

        ps2Write(l_port, s_ps2Ports[l_port].a_lastSentByte);
    } else {
        if(s_ps2Ports[l_port].a_receiveBufferLength == C_PS2_RECEIVE_BUFFER_SIZE) {
            return;
        }

        s_ps2Ports[l_port].a_receiveBuffer[s_ps2Ports[l_port].a_receiveBufferWriteIndex++] = l_value;
        s_ps2Ports[l_port].a_receiveBufferLength++;

        if(s_ps2Ports[l_port].a_receiveBufferWriteIndex == C_PS2_RECEIVE_BUFFER_SIZE) {
            s_ps2Ports[l_port].a_receiveBufferWriteIndex = 0;
        }
    }
}

static void ps2FlushReceiveBuffer(int p_port) {
    s_ps2Ports[p_port].a_receiveBufferWriteIndex = 0;
    s_ps2Ports[p_port].a_receiveBufferReadIndex = 0;
    s_ps2Ports[p_port].a_receiveBufferLength = 0;
}
