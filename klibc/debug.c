#include <stdarg.h>
#include <stddef.h>

#include "kernel/arch/x86_64/inline.h"
#include "klibc/stdio.h"

void kernelDebug(const char *p_str, ...) {
    va_list l_argList;
    char l_buffer[512];

    va_start(l_argList, p_str);

    kvsnprintf(l_buffer, 512, p_str, l_argList);

    va_end(l_argList);

    size_t l_index = 0;

    while(l_buffer[l_index]) {
        outb(0xe9, l_buffer[l_index++]);
    }
}