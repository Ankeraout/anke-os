#ifndef __INCLUDE_BOOT_BOOT_H__
#define __INCLUDE_BOOT_BOOT_H__

#include <stddef.h>
#include <stdint.h>

enum te_bootMemoryMapEntryType {
    E_MMAP_TYPE_FREE,
    E_MMAP_TYPE_RECLAIMABLE,
    E_MMAP_TYPE_KERNEL,
    E_MMAP_TYPE_RESERVED
};

struct ts_bootMemoryMapEntry {
    size_t base;
    size_t size;
    enum te_bootMemoryMapEntryType type;
};

struct ts_boot {
    size_t memoryMapLength;
    const struct ts_bootMemoryMapEntry *memoryMap;
};

extern void main(const struct ts_boot *p_boot);

#endif
