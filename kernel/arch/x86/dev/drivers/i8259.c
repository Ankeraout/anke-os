#include <stdbool.h>
#include <stdint.h>

#include <kernel/arch/x86/inline.h>
#include <kernel/dev/device.h>
#include <kernel/dev/interruptcontroller.h>
#include <kernel/common.h>
#include <kernel/debug.h>

#define C_IOPORT_I8259_MASTER_COMMAND 0x20
#define C_IOPORT_I8259_MASTER_DATA 0x21
#define C_IOPORT_I8259_SLAVE_COMMAND 0xa0
#define C_IOPORT_I8259_SLAVE_DATA 0xa1
#define C_I8259_CMD_ICW1_ICW4 0x01
#define C_I8259_CMD_ICW1_SINGLE 0x02
#define C_I8259_CMD_ICW1_INTERVAL4 0x04
#define C_I8259_CMD_ICW1_LEVEL 0x08
#define C_I8259_CMD_ICW1_INIT 0x10
#define C_I8259_CMD_ICW4_8086 0x01
#define C_I8259_CMD_ICW4_AUTO 0x02
#define C_I8259_CMD_ICW4_BUF_SLAVE 0x04
#define C_I8259_CMD_ICW4_BUF_MASTER 0x0c
#define C_I8259_CMD_ICW4_SFNM 0x10
#define C_I8259_CMD_EOI 0x20

static int i8259Init(struct ts_device *p_device);
static void i8259EndOfInterrupt(
    struct ts_device *p_device,
    int p_interruptNumber
);
static size_t i8259DriverApiGetChildCount(struct ts_device *p_device);
static struct ts_device *i8259DriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);

const struct ts_deviceDriverInterruptController g_deviceDriverI8259 = {
    .a_base = {
        .a_name = "Intel 8259 programmable interrupt controller",
        .a_api = {
            .a_init = i8259Init,
            .a_getChildCount = i8259DriverApiGetChildCount,
            .a_getChild = i8259DriverApiGetChild,
            .a_isSupported = NULL
        }
    },
    .a_api = {
        .a_endOfInterrupt = i8259EndOfInterrupt
    }
};

static int i8259Init(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    outb(C_IOPORT_I8259_MASTER_COMMAND, C_I8259_CMD_ICW1_INIT | C_I8259_CMD_ICW1_ICW4);
    iowait();
    outb(C_IOPORT_I8259_SLAVE_COMMAND, C_I8259_CMD_ICW1_INIT | C_I8259_CMD_ICW1_ICW4);
    iowait();
    outb(C_IOPORT_I8259_MASTER_DATA, 0x20); // Master 8259 vector offset (0x20-0x27)
    iowait();
    outb(C_IOPORT_I8259_SLAVE_DATA, 0x28); // Slave 8259 vector offset (0x28-0x2f)
    iowait();
    outb(C_IOPORT_I8259_MASTER_DATA, (1 << 2)); // Master 8259: Slave 8259 at IRQ2
    iowait();
    outb(C_IOPORT_I8259_SLAVE_DATA, (1 << 1)); // Slave 8259 cascade identity
    iowait();
    outb(C_IOPORT_I8259_MASTER_DATA, C_I8259_CMD_ICW4_8086);
    iowait();
    outb(C_IOPORT_I8259_SLAVE_DATA, C_I8259_CMD_ICW4_8086);
    iowait();
    outb(C_IOPORT_I8259_MASTER_DATA, 0x00); // Master 8259: Unmask all interrupts
    iowait();
    outb(C_IOPORT_I8259_SLAVE_DATA, 0x00); // Slave 8259: Unmask all interrupts
    iowait();

    debug("i8259: PIC initialized.\n");

    return 0;
}

static void i8259EndOfInterrupt(struct ts_device *p_device, int p_interruptNumber) {
    M_UNUSED_PARAMETER(p_device);

    if(p_interruptNumber >= 8) {
        outb(C_IOPORT_I8259_SLAVE_COMMAND, C_I8259_CMD_EOI);
        iowait();
    }

    outb(C_IOPORT_I8259_MASTER_COMMAND, C_I8259_CMD_EOI);
    iowait();
}

static size_t i8259DriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static struct ts_device *i8259DriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_index);

    return NULL;
}
