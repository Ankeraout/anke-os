#ifndef __KERNEL_LIBK_STDIO_H__
#define __KERNEL_LIBK_STDIO_H__

#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)

int puts(const char *str);
int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arguments);
int printf(const char *format, ...);

#endif
