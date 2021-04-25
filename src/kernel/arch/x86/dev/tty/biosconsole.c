#include <stdint.h>

#include "kernel/arch/x86/bios.h"

#include "kernel/libk/stdlib.h"

#include "kernel/sys/dev/tty.h"

void biosconsole_init();
static void biosconsole_write(tty_driver_t *driver, const char *str, size_t n);

void biosconsole_init() {
    tty_driver_t *driver = malloc(sizeof(tty_driver_t));

    driver->write = biosconsole_write;

    tty_driver_register(driver);
}

static void biosconsole_write(tty_driver_t *driver, const char *str, size_t n) {
    bios_callContext_t context = {
        .ah = 0x0e,
        .bh = 0,
        .bl = 0x07
    };
    
    for(size_t i = 0; i < n; i++) {
        context.al = str[i];

        bios_call(0x10, &context);
    }
}
