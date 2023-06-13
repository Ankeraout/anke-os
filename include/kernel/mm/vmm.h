#ifndef __INCLUDE_KERNEL_MM_VMM_H__
#define __INCLUDE_KERNEL_MM_VMM_H__

#include <stddef.h>

#include "kernel/boot.h"

#define C_VMM_ALLOC_USER 0x00000001
#define C_VMM_PROT_USER 0x00000001
#define C_VMM_PROT_WRITE 0x00000002
#define C_VMM_PROT_EXECUTE 0x00000004

int vmmInit(void);
void *vmmAlloc(size_t p_size, int p_flags);
void vmmFree(void *p_ptr, size_t p_size);
void vmmMap(void *p_pptr, void *p_vptr, size_t p_size);
void vmmUnmap(void *p_ptr, size_t p_size);
void vmmSetProtection(void *p_ptr, size_t p_size, int p_flags);

#endif
