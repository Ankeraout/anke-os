#ifndef __INCLUDE_MM_VMM_H__
#define __INCLUDE_MM_VMM_H__

#include <stddef.h>

#define C_VMM_PROT_EXEC 0x01
#define C_VMM_PROT_USER 0x02
#define C_VMM_PROT_WRITE 0x04

void vmmInit(void);
void *vmmAlloc(size_t p_size);
void vmmFree(void *p_ptr, size_t p_size);
void *vmmMap(void *p_pptr, void *p_vptr, size_t p_size, int p_prot);
void vmmUnmap(void *p_vptr, size_t p_size);
void *vmmProtect(void *p_ptr, size_t p_size, int p_prot);

#endif
