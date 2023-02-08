#ifndef __INCLUDE_DEV_PARALLEL_H__
#define __INCLUDE_DEV_PARALLEL_H__

#include <stdbool.h>
#include <stdint.h>

struct ts_devParallel {
    uint16_t a_basePort;
};

int parallelInit(struct ts_devParallel *p_dev);
void parallelSend(struct ts_devParallel *p_dev, uint8_t p_value);
uint8_t parallelReceive(struct ts_devParallel *p_dev);

#endif
