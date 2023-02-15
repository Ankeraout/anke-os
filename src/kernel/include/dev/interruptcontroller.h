#ifndef __INCLUDE_DEV_INTERRUPTCONTROLLER_H__
#define __INCLUDE_DEV_INTERRUPTCONTROLLER_H__

#include "dev/device.h"

struct ts_deviceDriverInterruptController;

typedef void tf_deviceDriverInterruptControllerFuncEndOfInterrupt(
    struct ts_device *p_device,
    int p_interruptNumber
);

struct ts_deviceDriverInterruptController {
    struct ts_deviceDriver a_base;
    struct {
        tf_deviceDriverInterruptControllerFuncEndOfInterrupt *a_endOfInterrupt;
    } a_api;
};

#endif
