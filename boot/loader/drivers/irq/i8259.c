#include "arch/x86/asm.h"
#include "arch/x86/irq.h"
#include "drivers/irq/irq_chip.h"

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

static void i8259_endOfInterrupt(int p_irq);
static void i8259_mask(int p_irq);
static void i8259_unmask(int p_irq);

static struct ts_irqChip s_i8259 = {
    .m_endOfInterrupt = i8259_endOfInterrupt,
    .m_mask = i8259_mask,
    .m_unmask = i8259_unmask
};

void i8259_init(void) {
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

    // Mask all interrupts by default (IRQ2 is not masked because it is the
    // slave PIC interrupt)
    outb(PIC1_DATA, 0xfb);
    outb(PIC2_DATA, 0xff);

    for(int l_irq = 32; l_irq < 48; l_irq++) {
        irqSetChip(l_irq, &s_i8259);
    }
}

static void i8259_endOfInterrupt(int p_irq) {
    if((p_irq < 32) || (p_irq >= 48)) {
        return;
    }

    if(p_irq >= 40) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

static void i8259_mask(int p_irq) {
    if((p_irq < 32) || (p_irq >= 48)) {
        return;
    }

    // IRQ2 cannot be masked
    if(p_irq == 34) {
        return;
    }

    uint16_t l_port;
    int l_shift;

    if(p_irq >= 40) {
        l_port = PIC2_DATA;
        l_shift = p_irq - 40;
    } else {
        l_port = PIC1_DATA;
        l_shift = p_irq - 32;
    }

    uint8_t l_mask = 1 << l_shift;

    outb(l_port, inb(l_port) | l_mask);
}

static void i8259_unmask(int p_irq) {
    if((p_irq < 32) || (p_irq >= 48)) {
        return;
    }

    uint16_t l_port;
    int l_shift;

    if(p_irq >= 40) {
        l_port = PIC2_DATA;
        l_shift = p_irq - 40;
    } else {
        l_port = PIC1_DATA;
        l_shift = p_irq - 32;
    }

    uint8_t l_mask = 1 << l_shift;

    outb(l_port, inb(l_port) & ~l_mask);
}
