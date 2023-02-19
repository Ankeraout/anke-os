#ifndef __INCLUDE_DEV_TIMER_H__
#define __INCLUDE_DEV_TIMER_H__

#include <stdint.h>

#include <kernel/dev/device.h>

struct ts_deviceDriverTimer;

typedef void tf_deviceDriverTimerFuncSetFrequency(
    struct ts_device *p_device,
    uint64_t p_frequency
);

typedef uint64_t tf_deviceDriverTimerFuncGetTime(
    struct ts_device *p_device
);

struct ts_deviceDriverTimer {
    struct ts_deviceDriver a_base;
    struct {
        tf_deviceDriverTimerFuncSetFrequency *a_setFrequency;
        tf_deviceDriverTimerFuncGetTime *a_getTime;
    } a_api;
};

void timerSetDevice(struct ts_device *p_device);
uint64_t timerGetTime(void);
void timerSleep(uint64_t p_milliseconds);

#endif
