#ifndef __INCLUDE_DEV_DEBUGCON_H__
#define __INCLUDE_DEV_DEBUGCON_H__

#include <stddef.h>
#include <stdint.h>

struct ts_devDebugcon {
    uint16_t a_basePort;
};

void debugconPutc(struct ts_devDebugcon *p_dev, uint8_t p_character);

#endif
