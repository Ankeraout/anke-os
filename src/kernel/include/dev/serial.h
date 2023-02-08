#ifndef __INCLUDE_DEV_SERIAL_H__
#define __INCLUDE_DEV_SERIAL_H__

#include <stdbool.h>
#include <stdint.h>

struct ts_devSerial {
    uint16_t a_basePort;
};

int serialInit(struct ts_devSerial *p_dev);
void serialSend(struct ts_devSerial *p_dev, uint8_t p_value);
uint8_t serialReceive(struct ts_devSerial *p_dev);

#endif
