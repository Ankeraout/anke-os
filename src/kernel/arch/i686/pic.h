#ifndef __PIC_H__
#define __PIC_H__

#define PIC1 0x20
#define PIC2 0xa0
#define PIC_EOI 0x20

void pic_init();
void pic_enableIRQ(int irqNumber);
void pic_disableIRQ(int irqNumber);
void pic_enableIRQs();
void pic_disableIRQs();

#endif
