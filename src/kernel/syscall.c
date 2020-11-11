#include <stdint.h>
#include <stddef.h>

#include "debug.h"
#include "libk/libk.h"
#include "syscall.h"

size_t syscall(size_t function, size_t argument) {
    char buffer[9];
    kernel_debug("syscall func=0x");
    kernel_debug(hex32(function, buffer));
    kernel_debug(" arg=0x");
    kernel_debug(hex32(argument, buffer));
    kernel_debug("\n");

    switch(function) {
        case 0: // malloc
            kernel_debug("calling kmalloc()... ");
            size_t result = (size_t)kmalloc(argument, false);
            kernel_debug("Done.\n");
            return result;

        default:
            return 0;
    }
}
