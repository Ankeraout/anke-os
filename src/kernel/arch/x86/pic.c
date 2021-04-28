#include "kernel/arch/x86/assembly.h"

#define PIC1 0x20
#define PIC2 0xa0
#define PIC_EOI 0x20

#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10
#define ICW4_8086 0x01

void pic_init();
void pic_disableIRQ(int irqNumber);
void pic_enableIRQ(int irqNumber);
void pic_disableIRQs();
void pic_enableIRQs();

void pic_init() {
    // Mask all interrupts
    pic_disableIRQs();

    // Send the init commands
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, 0x20); // PIC1: 0x20-0x27 interrupts
    io_wait();
    outb(PIC2_DATA, 0x28); // PIC2: 0x28-0x2f interrupts
    io_wait();
    outb(PIC1_DATA, (1 << 2)); // PIC1: PIC2 on interrupt line 2
    io_wait();
    outb(PIC2_DATA, 2); // PIC2: PIC1 on IRQ2
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Unmask all interrupts
    pic_enableIRQs();
}

void pic_disableIRQ(int irqNumber) {
    int pic;

    if(irqNumber >= 8) {
        pic = PIC2;
        irqNumber -= 8;
    } else {
        pic = PIC1;
    }
    
    outb(pic + 1, inb(pic + 1) | (1 << irqNumber));
}

void pic_enableIRQ(int irqNumber) {
    int pic;

    if(irqNumber >= 8) {
        pic = PIC2;
        irqNumber -= 8;
    } else {
        pic = PIC1;
    }

    outb(pic + 1, inb(pic + 1) & ~(1 << irqNumber));
}

void pic_disableIRQs() {
    outb(PIC2_DATA, 0xff);
    outb(PIC1_DATA, 0xff);
}

void pic_enableIRQs() {
    outb(PIC2_DATA, 0x00);
    outb(PIC1_DATA, 0x00);
}
