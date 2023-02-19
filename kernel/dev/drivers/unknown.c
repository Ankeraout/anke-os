#include <kernel/dev/device.h>
#include <kernel/common.h>

static int deviceUnknownDriverApiInit(struct ts_device *p_device);
static struct ts_device *deviceUnknownDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);
static size_t deviceUnknownDriverApiGetChildCount(struct ts_device *p_device);

const struct ts_deviceDriver g_deviceDriverUnknown = {
    .a_api = {
        .a_init = deviceUnknownDriverApiInit,
        .a_getChild = deviceUnknownDriverApiGetChild,
        .a_getChildCount = deviceUnknownDriverApiGetChildCount,
        .a_isSupported = NULL
    },
    .a_bus = E_DEVICEBUS_ANY,
    .a_name = "Unknown device"
};

static int deviceUnknownDriverApiInit(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static struct ts_device *deviceUnknownDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_index);

    return NULL;
}

static size_t deviceUnknownDriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}
