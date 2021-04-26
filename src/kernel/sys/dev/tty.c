#include <stddef.h>

#include "common.h"
#include "kernel/sys/dev/tty.h"

void tty_driver_register(tty_driver_t *tty_driver);
void tty_driver_input(tty_driver_t *driver, const void *str, size_t n);

static tty_driver_t *kernel_tty = NULL;

void tty_driver_register(tty_driver_t *driver) {
    if(kernel_tty == NULL) {
        kernel_tty = driver;
    }
}

void tty_driver_input(tty_driver_t *driver, const void *str, size_t n) {
    UNUSED_PARAMETER(driver);
    UNUSED_PARAMETER(str);
    UNUSED_PARAMETER(n);
}

tty_driver_t *tty_getDefault() {
    return kernel_tty;
}
