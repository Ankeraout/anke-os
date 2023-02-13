#include <stdint.h>

#include "arch/x86/inline.h"
#include "dev/device.h"
#include "dev/timer.h"
#include "debug.h"

static int i8254Init(struct ts_device *p_device);
static void i8254SetFrequency(struct ts_device *p_device, uint64_t a_frequency);

const struct ts_deviceDriverTimer g_devDriverI8254 = {
    .a_driver = {
        .a_name = "Intel 8253/8254 programmable interrupt timer",
        .a_init = (tf_deviceDriverFuncInit *)i8254Init
    },
    .a_setFrequency = i8254SetFrequency
};

struct ts_i8254DeviceDriverData {
    struct ts_deviceDriverTimerData a_timer;
};

static int i8254Init(struct ts_device *p_device) {
    debugPrint("i8254: init()\n");
    return 0;
}

static void i8254SetFrequency(
    struct ts_device *p_device,
    uint64_t a_frequency
) {

}
