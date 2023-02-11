#ifndef __INCLUDE_DEVICE_H__
#define __INCLUDE_DEVICE_H__

typedef int tf_deviceDriverInit(void);

struct ts_deviceDriver {
    
};

struct ts_device;

struct ts_device {
    struct ts_device *a_parent;
    struct ts_device **a_children;
    struct ts_deviceDriver *a_driver;
};

#endif
