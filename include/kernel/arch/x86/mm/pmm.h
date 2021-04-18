#ifndef __KERNEL_ARCH_X86_MM_PMM_H__
#define __KERNEL_ARCH_X86_MM_PMM_H__

void pmm_init();
void *pmm_alloc(int n);
void pmm_free(const void *pages, int n);

#endif
