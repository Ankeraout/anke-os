#ifndef __INCLUDE_DEV_IDE_H__
#define __INCLUDE_DEV_IDE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t t_devIdeLba;

enum te_devIdeDriveType {
    E_DEV_IDE_DRIVETYPE_NONE,
    E_DEV_IDE_DRIVETYPE_PATA,
    E_DEV_IDE_DRIVETYPE_PATAPI,
    E_DEV_IDE_DRIVETYPE_SATA,
    E_DEV_IDE_DRIVETYPE_SATAPI
};

enum te_devIdeDrive {
    E_DEV_IDE_DRIVE_MASTER,
    E_DEV_IDE_DRIVE_SLAVE
};

struct ts_devIdeDrive {
    enum te_devIdeDriveType a_type;
    uint16_t a_cylinders;
    uint8_t a_heads;
    uint8_t a_sectors;
    bool a_usesLba;
    bool a_usesLba48;
    t_devIdeLba a_sectorCount;
};

struct ts_devIde {
    uint16_t a_ioPortBase;
    uint16_t a_ioPortControl;
    int a_selectedDrive;
    uint8_t a_registerCacheDriveHead;
    uint8_t a_registerCacheStatus;
    struct ts_devIdeDrive a_drives[2];
};

int ideInit(struct ts_devIde *p_dev);
int ideRead(
    void *p_buffer,
    struct ts_devIde *p_dev,
    int p_drive,
    t_devIdeLba p_lba,
    size_t p_sectorCount
);

#endif
