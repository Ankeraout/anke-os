#include "dev/disk.h"

int fs_detectPartitionTable(dev_disk_t *disk);

typedef struct {
    uint8_t flags;
    uint8_t startingHead;
    uint16_t startingSector : 6;
    uint16_t startingCylinder : 10;
    uint8_t systemId;
    uint8_t endingHead;
    uint16_t endingSector : 6;
    uint16_t endingCylinder : 10;
    uint32_t startingLba;
    uint32_t totalSectorCount;
} __attribute__((packed)) partitionTableEntry_t;

typedef struct {
    uint8_t bootstrap[0x1b8];
    uint32_t diskUid;
    uint16_t reserved;
    partitionTableEntry_t partitionTable[4];
    uint16_t signature;
} __attribute__((packed)) mbr_t;

int fs_detectPartitionTable(dev_disk_t *disk) {
    mbr_t mbr;

    if(disk->partition) {
        return -1;
    }

    if(disk->blockSize != 512) {
        return 1;
    }

    if(disk->api.read(disk, &mbr, 0)) {
        return -2;
    }


}
