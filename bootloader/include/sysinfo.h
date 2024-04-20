#ifndef __INCLUDE_SYSINFO_H__
#define __INCLUDE_SYSINFO_H__

#include <stdint.h>

struct ts_systemInformation {
    uint64_t m_version; // The boot protocol version
    uint64_t m_bootloaderName; // Name and version of the bootloader
    uint64_t m_memoryMapAddress;
    uint64_t m_memoryMapSize; // Memory map size in bytes
    uint8_t m_pciSupported; // 0 = false, 1 = true
    uint8_t m_pciFlags; // See below
    uint8_t m_pciVersion[2]; // 2.2 BCD characters
    uint8_t m_lastPciBus;
    uint8_t m_unused[3];
    uint8_t m_bootDiskId[4];
    uint8_t m_bootVolumeId[4];
    uint8_t m_framebufferPresent; // 0 = false, 1 = true
    uint8_t m_vgaConsolePresent;  // 0 = false, 1 = true
    uint8_t m_unused2[6];
    uint64_t m_framebufferAddress;
    uint64_t m_framebufferWidth;
    uint64_t m_framebufferHeight;
    uint64_t m_framebufferPitch;
    uint8_t m_framebufferBpp;
    uint8_t m_framebufferRedShift;
    uint8_t m_framebufferRedBits;
    uint8_t m_framebufferGreenShift;
    uint8_t m_framebufferGreenBits;
    uint8_t m_framebufferBlueShift;
    uint8_t m_framebufferBlueBits;
    uint8_t m_unused3;
    uint16_t m_serialIoPorts[4]; // 0x0000 = no port
    uint16_t m_parallelIoPorts[4]; // 0x0000 = no port
} __attribute__((packed));

#endif
