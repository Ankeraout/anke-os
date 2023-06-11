#ifndef __INCLUDE_KERNEL_BOOT_H__
#define __INCLUDE_KERNEL_BOOT_H__

#include <stddef.h>
#include <stdint.h>

enum te_kernelMemoryMapEntryType {
    E_KERNELMEMORYMAPENTRYTYPE_FREE,
    E_KERNELMEMORYMAPENTRYTYPE_RESERVED,
    E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE
};

struct ts_kernelMemoryMapEntry {
    uint64_t base;
    uint64_t length;
    enum te_kernelMemoryMapEntryType type;
};

struct ts_kernelBootInfo {
    struct ts_kernelMemoryMapEntry *memoryMap;
    size_t memoryMapEntryCount;
};

#endif
