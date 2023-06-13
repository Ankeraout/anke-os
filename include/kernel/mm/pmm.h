#ifndef __INCLUDE_KERNEL_MM_PMM_H__
#define __INCLUDE_KERNEL_MM_PMM_H__

#include <stddef.h>

#include "kernel/boot.h"

int pmmInit(const struct ts_kernelBootInfo *p_bootInfo);
void *pmmAlloc(size_t p_size);
void pmmFree(void *p_ptr, size_t p_size);

#endif
