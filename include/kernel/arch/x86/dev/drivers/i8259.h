#ifndef __INCLUDE_ARCH_X86_DEV_DRIVERS_I8259_H__
#define __INCLUDE_ARCH_X86_DEV_DRIVERS_I8259_H__

#include <stdbool.h>

#include <kernel/dev/interruptcontroller.h>

extern const struct ts_deviceDriverInterruptController g_deviceDriverI8259;

#endif
