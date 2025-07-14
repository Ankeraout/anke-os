#ifndef __INCLUDE_ARCH_AMD64_PIC_H__
#define __INCLUDE_ARCH_AMD64_PIC_H__

void pic_init(void);
void pic_enableIrq(int p_irq);
void pic_disableIrq(int p_irq);
void pic_endOfInterrupt(int p_irq);

#endif
