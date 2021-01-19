#ifndef __KERNEL_ARCH_I686_PIC_H__
#define __KERNEL_ARCH_I686_PIC_H__

void pic_init();
void pic_enableIRQ(int irqNumber);
void pic_disableIRQ(int irqNumber);
void pic_enableIRQs();
void pic_disableIRQs();

#endif
