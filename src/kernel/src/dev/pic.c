#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "debug.h"

#define C_IOPORT_PIC1_COMMAND 0x20
#define C_IOPORT_PIC1_DATA 0x21
#define C_IOPORT_PIC2_COMMAND 0xa0
#define C_IOPORT_PIC2_DATA 0xa1
#define C_PIC_CMD_ICW1_ICW4 0x01
#define C_PIC_CMD_ICW1_SINGLE 0x02
#define C_PIC_CMD_ICW1_INTERVAL4 0x04
#define C_PIC_CMD_ICW1_LEVEL 0x08
#define C_PIC_CMD_ICW1_INIT 0x10
#define C_PIC_CMD_ICW4_8086 0x01
#define C_PIC_CMD_ICW4_AUTO 0x02
#define C_PIC_CMD_ICW4_BUF_SLAVE 0x04
#define C_PIC_CMD_ICW4_BUF_MASTER 0x0c
#define C_PIC_CMD_ICW4_SFNM 0x10
#define C_PIC_CMD_EOI 0x20

void picInit(void) {
    outb(C_IOPORT_PIC1_COMMAND, C_PIC_CMD_ICW1_INIT | C_PIC_CMD_ICW1_ICW4);
    iowait();
    outb(C_IOPORT_PIC2_COMMAND, C_PIC_CMD_ICW1_INIT | C_PIC_CMD_ICW1_ICW4);
    iowait();
    outb(C_IOPORT_PIC1_DATA, 0x20); // Master PIC vector offset (0x20-0x27)
    iowait();
    outb(C_IOPORT_PIC2_DATA, 0x28); // Slave PIC vector offset (0x28-0x2f)
    iowait();
    outb(C_IOPORT_PIC1_DATA, (1 << 2)); // Master PIC: Slave PIC at IRQ2
    iowait();
    outb(C_IOPORT_PIC2_DATA, (1 << 1)); // Slave PIC cascade identity
    iowait();
    outb(C_IOPORT_PIC1_DATA, C_PIC_CMD_ICW4_8086);
    iowait();
    outb(C_IOPORT_PIC2_DATA, C_PIC_CMD_ICW4_8086);
    iowait();
    outb(C_IOPORT_PIC1_DATA, 0x00); // Master PIC: Unmask all interrupts
    iowait();
    outb(C_IOPORT_PIC2_DATA, 0x00); // Slave PIC: Unmask all interrupts
    iowait();

    debugPrint("pic: PIC initialized.\n");
}

void picEndOfInterrupt(bool l_slaveInterrupt) {
    if(l_slaveInterrupt) {
        outb(C_IOPORT_PIC2_COMMAND, C_PIC_CMD_EOI);
        iowait();
    }

    outb(C_IOPORT_PIC1_COMMAND, C_PIC_CMD_EOI);
    iowait();
}
