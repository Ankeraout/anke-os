#include <stdarg.h>

#include "kernel/panic.h"
#include "kernel/printk.h"

void panic(const char *p_format, ...) {
    pr_alert("panic: ");

    va_list l_args;
    va_start(l_args, p_format);
    vprintk(p_format, l_args);
    va_end(l_args);

    while(1);
}
