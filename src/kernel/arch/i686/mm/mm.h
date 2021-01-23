#ifndef __KERNEL_ARCH_I686_MM_MM_H__
#define __KERNEL_ARCH_I686_MM_MM_H__

#include <stddef.h>

typedef enum {
    MM_SERVICE_PMM,
    MM_SERVICE_VMM
} mm_service_t;

void mm_init();
void *mm_mapTemporary(const void *pageAddress, mm_service_t service);
void mm_unmapTemporary(mm_service_t service);
void *mm_alloc(size_t n);
void mm_free(void *buffer, size_t n);

#endif
