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
    uint64_t m_base;
    uint64_t m_size;
    enum te_kernelMemoryMapEntryType m_type;
};

struct ts_bootFramebufferInfo {
    void *m_address;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_pitch;
    unsigned int m_bitsPerPixel;
    unsigned int m_redMaskSize;
    unsigned int m_redMaskShift;
    unsigned int m_greenMaskSize;
    unsigned int m_greenMaskShift;
    unsigned int m_blueMaskSize;
    unsigned int m_blueMaskShift;
};

struct ts_kernelBootInfo {
    struct ts_kernelMemoryMapEntry *memoryMap;
    size_t memoryMapEntryCount;
    struct ts_bootFramebufferInfo m_framebufferInfo;
};

const struct ts_kernelBootInfo *kernelGetBootInfo(void);

#endif
