#include "arch/arch.h"
#include "dev/timer.h"

#define C_TIMER_FREQUENCY 100
#define C_TIMER_PERIOD (1000 / C_TIMER_FREQUENCY)

static struct ts_device *s_timerDevice;

void timerSetDevice(struct ts_device *p_device) {
    s_timerDevice = p_device;

    const struct ts_deviceDriverTimer *l_driver =
        (const struct ts_deviceDriverTimer *)s_timerDevice->a_driver;

    l_driver->a_api.a_setFrequency(s_timerDevice, C_TIMER_FREQUENCY);
}

uint64_t timerGetTime(void) {
    const struct ts_deviceDriverTimer *l_driver =
        (const struct ts_deviceDriverTimer *)s_timerDevice->a_driver;

    return l_driver->a_api.a_getTime(s_timerDevice);
}

void timerSleep(uint64_t p_milliseconds) {
    const struct ts_deviceDriverTimer *l_driver =
        (const struct ts_deviceDriverTimer *)s_timerDevice->a_driver;

    if(p_milliseconds == 0) {
        return;
    }

    uint64_t l_sleepEndTime =
        l_driver->a_api.a_getTime(s_timerDevice) + p_milliseconds + C_TIMER_PERIOD;

    while(l_driver->a_api.a_getTime(s_timerDevice) < l_sleepEndTime) {
        archHalt();
    }
}
