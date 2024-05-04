#include "kernel/printk.h"

void printk(const char *p_format, ...) {
    va_list l_args;
    va_start(l_args, p_format);

    vprintk(p_format, l_args);

    va_end(l_args);
}
