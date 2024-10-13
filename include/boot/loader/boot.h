#ifndef __INCLUDE_BOOT_LOADER_BOOT_H__
#define __INCLUDE_BOOT_LOADER_BOOT_H__

#include <stdint.h>

struct ts_bootInfoStructure {
    uint32_t m_memoryMapAddress;
    uint32_t m_memoryMapSize;
    uint32_t m_framebufferAddress;
    uint32_t m_framebufferWidth;
    uint32_t m_framebufferHeight;
    uint32_t m_framebufferPitch;
    uint32_t m_framebufferBpp;
    uint32_t m_framebufferRedBits;
    uint32_t m_framebufferRedShift;
    uint32_t m_framebufferGreenBits;
    uint32_t m_framebufferGreenShift;
    uint32_t m_framebufferBlueBits;
    uint32_t m_framebufferBlueShift;
    uint8_t m_bootDrive;
    uint8_t m_pciSupported;
} __attribute__((packed));

#endif
