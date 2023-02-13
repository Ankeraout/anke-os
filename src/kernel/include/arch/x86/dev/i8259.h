#ifndef __INCLUDE_ARCH_X86_DEV_I8259_H__
#define __INCLUDE_ARCH_X86_DEV_I8259_H__

#include <stdbool.h>

void i8259Init(void);
void i8259EndOfInterrupt(bool l_slaveInterrupt);

#endif
