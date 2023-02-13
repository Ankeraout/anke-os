#include "arch/x86/inline.h"
#include "dev/device.h"
#include "common.h"
#include "debug.h"

#define C_IOPORT_PS2_DATA 0x60
#define C_IOPORT_PS2_COMMAND 0x64
#define C_IOPORT_PS2_STATUS 0x64

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
#define C_PS2_RECEIVE_BUFFER_SIZE 16

#define C_PS2_TIMEOUT_RESET 1000
#define C_PS2_TIMEOUT_ACK 100

static int i8042Init(struct ts_device *p_device);

const struct ts_deviceDriver g_devDriverI8042 = {
    .a_name = "8042 PS/2 controller",
    .a_init = i8042Init
};

static int i8042Init(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    debugPrint("i8042: init()\n");

    return 0;
}
