#include <stdint.h>
#include <stddef.h>

#include "debug.h"
#include "libk/libk.h"
#include "syscall.h"

void syscall(size_t function, size_t argument) {
    char buffer[9];
    kernel_debug("Syscall! Func=");
    kernel_debug(hex32(function, buffer));
    kernel_debug(" Arg=");
    kernel_debug(hex32(argument, buffer));
    kernel_debug("\n");
}
