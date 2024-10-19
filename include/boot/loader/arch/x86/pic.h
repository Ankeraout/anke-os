#ifndef __INCLUDE_BOOT_LOADER_ARCH_X86_PIC_H__
#define __INCLUDE_BOOT_LOADER_ARCH_X86_PIC_H__

void pic_init(void);
void pic_endOfInterrupt(int p_irq);

#endif
