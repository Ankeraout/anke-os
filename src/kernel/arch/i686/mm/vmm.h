#ifndef __KERNEL_ARCH_I686_MM_VMM_H__
#define __KERNEL_ARCH_I686_MM_VMM_H__

#include <stdbool.h>
#include <stddef.h>

void vmm_init();
void *vmm_map(const void *paddr, size_t n, bool high);
void *vmm_map2(const void *paddr, void *vaddr, size_t n);
int vmm_unmap(const void *vaddr, size_t n);

#endif
