#include <stdbool.h>
#include <stddef.h>

#include "arch/arch.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "arch/x86/dev/acpi.h"
#include "dev/device.h"
#include "debug.h"

static struct ts_device s_acpiDevice = {
    .a_driver = &g_deviceDriverAcpi,
    .a_parent = NULL,
    .a_driverData = NULL
};

void archInit(void) {
    gdtInit();
    idtInit();

    if(s_acpiDevice.a_driver->a_init(&s_acpiDevice)) {
        debugPrint("kernel: ACPI driver initialization failed.\n");
        debugPrint("kernel: System halted.\n");
        archHaltAndCatchFire();
    }
}

void archInterruptsEnable(void) {
    sti();
}

void archInterruptsDisable(void) {
    cli();
}

void archHalt(void) {
    hlt();
}

void archHaltAndCatchFire(void) {
    while(true) {
        cli();
        hlt();
    }
}
