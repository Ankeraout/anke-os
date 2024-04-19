# bootstrap16
bootstrap16 has a few roles:
- Acquire as much information as possible about the system it's running on,
using BIOS calls.
- Initialize the system (framebuffer, ...)
- Prepare the system information structure
- Change the CPU mode to 32-bit protected mode and jump to bootstrap32.

# Memory map
Here is the memory map after bootstrap16 is done:
|Start address|End address|Size|Content|
|-|-|-|-|
|0x00000000|0x000003ff|0x00000000|Real-mode IVT|
|0x00000400|0x000004ff|0x00000100|BDA|
|0x00000500|0x00000fff|0x00000b00|Unused|
|0x00001000|0x00001fff|0x00001000|System information structure|
|0x00002000|0x00002000|0x00001000|System memory map|
|0x00003000|0x00009fff|0x00007000|Unused|
|0x00010000|0x0001xxxx|0x0000xxxx|bootstrap16|
|0x000xxxxx|0x000xxxxx|0x000xxxxx|bootstrap32|
|0x000xxxxx|0x000xxxxx|0x000xxxxx|bootstrap64|
|0x000xxxxx|0x000xxxxx|0x000xxxxx|Bootloader code|
|0x0008f000|0x0009efff|0x00010000|bootstrap16's stack|
|0x0009f000|0x0009ffff|0x00001000|Reserved|
|0x000a0000|0x000bffff|0x00020000|VGA memory|
|0x000c0000|0x000fffff|0x00040000|BIOS|

**NOTE:** bootstrap16's memory can be reclaimed after its execution is done,
including its stack.

**NOTE:** Note that the system information structure may be placed anywhere in
the unused area 

## System memory map
The system memory map is an array of 24 bytes-long entries.
These entries follow this structure:
```c
struct ts_memoryMapEntry {
    uint64_t m_address;
    uint64_t m_size;
    uint32_t m_type;
    uint32_t m_flags;
} __attribute__((packed));
```

## System information structure
The system information structure is a data structure that is placed in memory
by bootstrap16 for use by following bootloaders. The address of this structure
is stored in EBX when bootstrap32 is called.

### Version 1
Its structure is as follows:
```c
struct ts_bootloader_info {
    uint64_t m_version; // The boot protocol version
    const uint8_t *m_bootloaderName; // Name and version of the bootloader
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
```

#### PCI flags
|Bit|Meaning|
|-|-|
|0|CSAM 1 supported|
|1|CSAM 2 supported|
|2|Unused|
|3|Unused|
|4|Special cycle generation mechanism 1 supported|
|5|Special cycle generation mechanism 2 supported|
|6|Unused|
|7|Unused|
