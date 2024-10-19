#ifndef __INCLUDE_BOOT_LOADER_DRIVERS_CONSOLE_CONSOLE_H__
#define __INCLUDE_BOOT_LOADER_DRIVERS_CONSOLE_CONSOLE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "boot/loader/types.h"

struct ts_console {
    // Used by the console subsystem, do not modify in driver code.
    int id;
    bool enabled;

    // Filled by the console driver
    void *driverData;
    ssize_t (*write)(
        struct ts_console *p_console,
        const void *p_buffer,
        size_t p_size
    );
};

void console_init(void);
struct ts_console *console_alloc(void);
int console_register(struct ts_console *p_console);
ssize_t console_write(const void *p_buffer, size_t p_size);

#endif
