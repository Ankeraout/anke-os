#ifndef __INCLUDE_KERNEL_ARCH_X86_64_PIC_H__
#define __INCLUDE_KERNEL_ARCH_X86_64_PIC_H__

void picInit(void);
void picEnableIrq(int p_irq);
void picDisableIrq(int p_irq);
void picEndOfInterrupt(int p_irq);

#endif
