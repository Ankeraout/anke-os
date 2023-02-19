#ifndef __INCLUDE_MM_PMM_H__
#define __INCLUDE_MM_PMM_H__

#include <stddef.h>

#include <kernel/boot/boot.h>

int pmmInit(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength
);
void *pmmAlloc(size_t p_size);
void pmmFree(void *p_ptr, size_t p_size);

#endif
