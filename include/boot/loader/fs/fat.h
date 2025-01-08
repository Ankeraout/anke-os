#ifndef __INCLUDE_BOOT_LOADER_FS_FAT_H__
#define __INCLUDE_BOOT_LOADER_FS_FAT_H__

#include <stdint.h>

struct ts_fatBpbCommon {
    uint8_t m_jump[3];
    uint8_t m_oem[8];
    uint16_t m_bytesPerSector;
    uint8_t m_sectorsPerCluster;
    uint16_t m_reservedSectors;
    uint8_t m_fatCount;
    uint16_t m_rootEntryCount;
    uint16_t m_totalSectors16;
    uint8_t m_media;
    uint16_t m_fatSize16;
    uint16_t m_sectorsPerTrack;
    uint16_t m_headCount;
    uint32_t m_hiddenSectors;
    uint32_t m_totalSectors32;
} __attribute__((packed));

struct ts_fatBpb1216 {
    struct ts_fatBpbCommon m_common;
    uint8_t m_driveNumber;
    uint8_t m_reserved;
    uint8_t m_signature;
    uint32_t m_volumeId;
    uint8_t m_volumeLabel[11];
    uint8_t m_systemId[8];
} __attribute__((packed));

struct ts_fatBpb32 {
    struct ts_fatBpbCommon m_common;
    uint32_t m_fatSize32;
    uint16_t m_extFlags;
    uint16_t m_fsVersion;
    uint32_t m_rootCluster;
    uint16_t m_fsInfo;
    uint16_t m_backupBootSector;
    uint8_t m_reserved[12];
    uint8_t m_driveNumber;
    uint8_t m_reserved1;
    uint8_t m_signature;
    uint32_t m_volumeId;
    uint8_t m_volumeLabel[11];
    uint8_t m_systemId[8];
} __attribute__((packed));

struct ts_fatFsInfo {
    uint32_t m_leadSig;
    uint8_t m_reserved1[480];
    uint32_t m_strucSig;
    uint32_t m_freeCount;
    uint32_t m_nextFree;
    uint8_t m_reserved2[12];
    uint32_t m_trailSig;
} __attribute__((packed));

struct ts_fatDirEntry {
    uint8_t m_name[11];
    uint8_t m_attr;
    uint8_t m_reserved;
    uint8_t m_crtTimeTenth;
    uint16_t m_crtTime;
    uint16_t m_crtDate;
    uint16_t m_lastAccessDate;
    uint16_t m_firstClusterHigh;
    uint16_t m_writeTime;
    uint16_t m_writeDate;
    uint16_t m_firstClusterLow;
    uint32_t m_fileSize;
} __attribute__((packed));

#endif
