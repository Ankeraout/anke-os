#ifndef __INCLUDE_KERNEL_ARCH_ARCH_H__
#define __INCLUDE_KERNEL_ARCH_ARCH_H__

#include "kernel/boot.h"

int archPreInit(const struct ts_kernelBootInfo *p_bootInfo);
int archInit(void);
int archPostInit(void);

#endif
