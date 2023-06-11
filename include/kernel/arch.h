#ifndef __INCLUDE_KERNEL_ARCH_H__
#define __INCLUDE_KERNEL_ARCH_H__

#include "kernel/boot.h"

int archPreinit(const struct ts_kernelBootInfo *p_bootInfo);
int archInit(void);

#endif
