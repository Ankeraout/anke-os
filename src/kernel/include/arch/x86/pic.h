#ifndef __INCLUDE_ARCH_X86_PIC_H__
#define __INCLUDE_ARCH_X86_PIC_H__

#include <stdbool.h>

void picInit(void);
void picEndOfInterrupt(bool l_slaveInterrupt);

#endif
