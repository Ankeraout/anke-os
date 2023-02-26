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
    uint64_t a_base;
    uint64_t a_size;
    enum te_bootMemoryMapEntryType a_type;
};

struct ts_boot {
    size_t a_memoryMapLength;
    const struct ts_bootMemoryMapEntry *a_memoryMap;
};

extern void main(struct ts_boot *p_boot);

#endif
