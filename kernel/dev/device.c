#include <kernel/dev/device.h>
#include <kernel/dev/drivers/unknown.h>
#include <kernel/misc/list.h>

static struct ts_arrayList s_driverList;

int deviceInit(void) {
    if(arrayListInit(&s_driverList) != 0) {
        return 1;
    } else {
        return 0;
    }
}

const struct ts_deviceDriver *deviceGetDriver(
    const struct ts_deviceIdentifier *p_deviceIdentifier
) {
    size_t l_driverCount = arrayListGetLength(&s_driverList);

    for(size_t l_index = 0; l_index < l_driverCount; l_index++) {
        const struct ts_deviceDriver *l_driver =
            (const struct ts_deviceDriver *)arrayListGet(&s_driverList, l_index);

        if(
            (l_driver->a_api.a_isSupported != NULL)
            && (l_driver->a_api.a_isSupported(p_deviceIdentifier))
        ) {
            return l_driver;
        }
    }

    return &g_deviceDriverUnknown;
}

int deviceRegisterDriver(const struct ts_deviceDriver *p_driver) {
    if(arrayListAdd(&s_driverList, (struct ts_deviceDriver *)p_driver) != 0) {
        return 1;
    }

    return 0;
}
