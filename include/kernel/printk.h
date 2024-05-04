#ifndef __INCLUDE_KERNEL_PRINTK_H__
#define __INCLUDE_KERNEL_PRINTK_H__

#include <stdarg.h>

void printk(const char *p_format, ...);
void vprintk(const char *p_format, va_list p_args);

#endif
