#ifndef __INCLUDE_KERNEL_SPRINTF_H__
#define __INCLUDE_KERNEL_SPRINTF_H__

#include <stdarg.h>
#include <stddef.h>

int sprintf(char *p_buffer, const char *p_format, ...);
int vsprintf(char *p_buffer, const char *p_format, va_list p_args);
int snprintf(char *p_buffer, size_t p_size, const char *p_format, ...);
int vsnprintf(
    char *p_buffer,
    size_t p_size,
    const char *p_format,
    va_list p_args
);

#endif
