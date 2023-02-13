#ifndef __INCLUDE_DEV_TIMER_H__
#define __INCLUDE_DEV_TIMER_H__

#include <stdint.h>

#include "dev/device.h"

struct ts_deviceDriverTimer;

typedef void tf_deviceDriverTimerFuncSetFrequency(
    struct ts_device *p_device,
    uint64_t p_frequency
);

struct ts_deviceDriverTimer {
    struct ts_deviceDriver a_driver;
    tf_deviceDriverTimerFuncSetFrequency *a_setFrequency;
};

struct ts_deviceDriverTimerData {
    uint64_t a_milliseconds;
};

#endif
