#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "debug.h"

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

void i8259Init(void) {
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

    debugPrint("i8259: PIC initialized.\n");
}

void i8259EndOfInterrupt(bool l_slaveInterrupt) {
    if(l_slaveInterrupt) {
        outb(C_IOPORT_I8259_SLAVE_COMMAND, C_I8259_CMD_EOI);
        iowait();
    }

    outb(C_IOPORT_I8259_MASTER_DATA, C_I8259_CMD_EOI);
    iowait();
}
