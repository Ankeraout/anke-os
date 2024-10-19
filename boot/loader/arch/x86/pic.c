#include <stdint.h>

#include "boot/loader/arch/x86/asm.h"

#define C_PIC1_PORT_COMMAND 0x20
#define C_PIC1_PORT_DATA 0x21
#define C_PIC2_PORT_COMMAND 0xa0
#define C_PIC2_PORT_DATA 0xa1

#define C_PIC_COMMAND_EOI 0x20
#define C_PIC_ICW1_ICW4 0x01
#define C_PIC_ICW1_SINGLE 0x02
#define C_PIC_ICW1_INTERVAL4 0x04
#define C_PIC_ICW1_LEVEL 0x08
#define C_PIC_ICW1_INIT 0x10
#define C_PIC_ICW4_8086 0x01
#define C_PIC_ICW4_AUTO 0x02
#define C_PIC_ICW4_BUF_SLAVE 0x08
#define C_PIC_ICW4_BUF_MASTER 0x0c
#define C_PIC_ICW4_SFNM 0x10

void pic_init(void) {
    // Mask all interrupts
    outb(C_PIC1_PORT_DATA, 0xff);
    outb(C_PIC2_PORT_DATA, 0xff);

    // Start initialization sequence in cascade mode
    outb(C_PIC1_PORT_COMMAND, C_PIC_ICW1_INIT | C_PIC_ICW1_ICW4);
    iowait();
    outb(C_PIC2_PORT_COMMAND, C_PIC_ICW1_INIT | C_PIC_ICW1_ICW4);
    iowait();
    outb(C_PIC1_PORT_DATA, 0x20);
    iowait();
    outb(C_PIC2_PORT_DATA, 0x28);
    iowait();
    outb(C_PIC1_PORT_DATA, 4);
    iowait();
    outb(C_PIC2_PORT_DATA, 2);
    iowait();
    outb(C_PIC1_PORT_DATA, C_PIC_ICW4_8086);
    iowait();
    outb(C_PIC2_PORT_DATA, C_PIC_ICW4_8086);
    iowait();

    // Unmask all interrupts
    outb(C_PIC1_PORT_DATA, 0x00);
    outb(C_PIC2_PORT_DATA, 0x00);
}

void pic_endOfInterrupt(int p_irq) {
    if(p_irq >= 8) {
        outb(C_PIC2_PORT_COMMAND, C_PIC_COMMAND_EOI);
    }

    outb(C_PIC1_PORT_COMMAND, C_PIC_COMMAND_EOI);
}
