#include <stdint.h>

#include "arch/amd64/asm.h"
#include "printk.h"

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

void picInit(void) {
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
    outb(C_PIC1_PORT_DATA, C_PIC_ICW4_8086);
    iowait();

    // Mask all interrupts
    outb(C_PIC1_PORT_DATA, 0xff);
    outb(C_PIC2_PORT_DATA, 0xff);

    pr_info("pic: PIC initialized.\n");
}

void picEnableIrq(int p_irq) {
    uint16_t l_port;
    uint8_t l_mask;

    if(p_irq < 0 || p_irq > 15) {
        return;
    }

    if(p_irq >= 8) {
        l_mask = (1 << (p_irq - 8));
        l_port = C_PIC2_PORT_DATA;
    } else {
        l_mask = 1 << p_irq;
        l_port = C_PIC1_PORT_DATA;
    }

    outb(l_port, inb(l_port) & ~l_mask);
}

void picDisableIrq(int p_irq) {
    uint16_t l_port;
    uint8_t l_mask;

    if(p_irq < 0 || p_irq > 15) {
        return;
    }

    if(p_irq >= 8) {
        l_mask = (1 << (p_irq - 8));
        l_port = C_PIC2_PORT_DATA;
    } else {
        l_mask = 1 << p_irq;
        l_port = C_PIC1_PORT_DATA;
    }

    outb(l_port, inb(l_port) | l_mask);
}

void picEndOfInterrupt(int p_irq) {
    if(p_irq >= 8) {
        outb(C_PIC2_PORT_COMMAND, C_PIC_COMMAND_EOI);
    }

    outb(C_PIC1_PORT_COMMAND, C_PIC_COMMAND_EOI);
}
