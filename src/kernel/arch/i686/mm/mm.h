#ifndef __KERNEL_ARCH_I686_MM_MM_H__
#define __KERNEL_ARCH_I686_MM_MM_H__

typedef enum {
    MM_SERVICE_PMM,
    MM_SERVICE_VMM,
    MM_SERVICE_E820
} mm_service_t;

void mm_init();
void *mm_mapTemporary(const void *pageAddress, mm_service_t service);
void mm_unmapTemporary(mm_service_t service);

#endif
