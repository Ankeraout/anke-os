#ifndef __INCLUDE_BOOT_LOADER_DRIVERS_CMOS_H__
#define __INCLUDE_BOOT_LOADER_DRIVERS_CMOS_H__

#include <stdint.h>

#define C_CMOS_REGISTER_FLOPPY 0x10

uint8_t cmos_read(uint8_t p_register);

#endif
