#include <stdbool.h>
#include <stdint.h>

#include "dev/disk/disk.h"

typedef struct {
    uint8_t driveAttributes;
    uint8_t startHead;
    uint16_t startSector : 6;
    uint16_t startCylinder : 10;
    uint8_t endHead;
    uint16_t endSector : 6;
    uint16_t endCylinder : 10;
    uint32_t lbaStart;
    uint32_t lbaTotal;
} mbr_partitionEntry_t;

void disk_partition_discover(dev_disk_t *disk);
bool disk_partition_isEntryValid(dev_disk_t *disk, mbr_partitionEntry_t *partitionEntry);

void disk_partition_discover(dev_disk_t *disk) {
    if(disk->blockSize != 512) {
        return;
    }

    uint8_t mbr[512];

    disk->api.read(disk, mbr, 0);

    
}

bool disk_partition_isEntryValid(dev_disk_t *disk, mbr_partitionEntry_t *partitionEntry) {
    return false;
}
