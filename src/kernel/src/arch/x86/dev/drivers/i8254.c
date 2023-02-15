#include <stdint.h>

#include "arch/x86/inline.h"
#include "arch/x86/isr.h"
#include "dev/device.h"
#include "dev/timer.h"
#include "klibc/stdlib.h"
#include "common.h"
#include "debug.h"

#define C_8254_INTERRUPT_NUMBER 32
#define C_8254_FREQUENCY 1193182
#define C_8254_IOPORT_C0_DATA 0x40
#define C_8254_IOPORT_C1_DATA 0x41
#define C_8254_IOPORT_C2_DATA 0x42
#define C_8254_IOPORT_CMD 0x43

struct ts_i8254Data {
    uint64_t a_time;
    uint64_t a_period;
};

struct ts_i8254Data s_i8254Data;

static int i8254Init(struct ts_device *p_device);
static uint64_t i8254GetTime(struct ts_device *p_device);
static void i8254SetFrequency(struct ts_device *p_device, uint64_t p_frequency);
static void i8254InterruptHandler(
    struct ts_isrRegisters *p_registers,
    struct ts_device *p_device
);
static size_t i8254DriverApiGetChildCount(struct ts_device *p_device);
static struct ts_device *i8254DriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);

const struct ts_deviceDriverTimer g_deviceDriverI8254 = {
    .a_base = {
        .a_name = "Intel 8253/8254 programmable interrupt timer",
        .a_api = {
            .a_init = (tf_deviceDriverApiInit *)i8254Init,
            .a_getChild = i8254DriverApiGetChild,
            .a_getChildCount = i8254DriverApiGetChildCount,
            .a_isSupported = NULL
        }
    },
    .a_api = {
        .a_setFrequency = i8254SetFrequency,
        .a_getTime = i8254GetTime
    }
};

static int i8254Init(struct ts_device *p_device) {
    p_device->a_driverData = kmalloc(sizeof(struct ts_i8254Data));

    if(p_device->a_driverData == NULL) {
        debugPrint("i8254: Failed to allocate memory for driver data.\n");
        return 1;
    }

    ((struct ts_i8254Data *)p_device->a_driverData)->a_time = 0;

    isrSetHandler(
        C_8254_INTERRUPT_NUMBER,
        (tf_isrHandler *)i8254InterruptHandler,
        p_device
    );

    outb(C_8254_IOPORT_CMD, 0x36);

    debugPrint("i8254: PIT Initialized.\n");

    return 0;
}

static uint64_t i8254GetTime(struct ts_device *p_device) {
    return ((const struct ts_i8254Data *)p_device->a_driverData)->a_time;
}

static void i8254SetFrequency(
    struct ts_device *p_device,
    uint64_t p_frequency
) {
    struct ts_i8254Data *l_data = (struct ts_i8254Data *)p_device->a_driverData;

    uint64_t l_periodTicks = C_8254_FREQUENCY / p_frequency;

    outb(C_8254_IOPORT_C0_DATA, l_periodTicks);
    outb(C_8254_IOPORT_C0_DATA, l_periodTicks >> 8);

    l_data->a_period = 1000 / p_frequency;
}

static void i8254InterruptHandler(
    struct ts_isrRegisters *p_registers,
    struct ts_device *p_device
) {
    M_UNUSED_PARAMETER(p_registers);

    struct ts_i8254Data *l_data = (struct ts_i8254Data *)p_device->a_driverData;

    l_data->a_time += l_data->a_period;
}

static size_t i8254DriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static struct ts_device *i8254DriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_index);

    return NULL;
}