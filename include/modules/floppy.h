#ifndef __INCLUDE_MODULES_FLOPPY_H__
#define __INCLUDE_MODULES_FLOPPY_H__

#include <stdint.h>

enum {
    E_IOCTL_FLOPPY_CREATE = 1
};

struct ts_floppyGeometry {
    uint16_t a_cylinders;
    uint8_t a_sectors;
};

struct ts_floppyRequestCreate {
    struct ts_floppyGeometry a_geometry;
    int a_driveNumber;
};

#endif
