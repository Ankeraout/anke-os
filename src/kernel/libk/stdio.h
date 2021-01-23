#ifndef __KERNEL_LIBK_STDIO_H__
#define __KERNEL_LIBK_STDIO_H__

#include <stddef.h>
#include <stdarg.h>

int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arguments);
int printf(const char *format, ...);

#endif
