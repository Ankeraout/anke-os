#include <stddef.h>

#include "kernel/libk/string.h"

#include "kernel/sys/dev/tty.h"

int puts(const char *str);

int puts(const char *str) {
    const tty_driver_t *tty_driver = tty_getDefault();

    if(tty_driver != NULL) {
        tty_driver->write(tty_driver, str, strlen(str));
    }

    return 0;
}
