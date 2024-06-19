#include "arch/x86/asm.h"
#include "arch/x86/pic.h"

#define PIC1            0x20
#define PIC2            0xA0
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define PIC_EOI         0x20
#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10
#define ICW4_8086       0x01
#define ICW4_AUTO       0x02
#define ICW4_BUF_SLAVE  0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM       0x10

void picInit(void) {
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    iowait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    iowait();
    outb(PIC1_DATA, 0x20);
    iowait();
    outb(PIC2_DATA, 0x28);
    iowait();
    outb(PIC1_DATA, 4);
    iowait();
    outb(PIC2_DATA, 2);
    iowait();
    outb(PIC1_DATA, ICW4_8086);
    iowait();
    outb(PIC2_DATA, ICW4_8086);
    iowait();
}

void picEndOfInterrupt(bool p_slave) {
    if(p_slave) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}
