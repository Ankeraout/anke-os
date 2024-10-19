#ifndef __INCLUDE_BOOT_LOADER_STDIO_H__
#define __INCLUDE_BOOT_LOADER_STDIO_H__

#include <stdarg.h>
#include <stddef.h>

int sprintf(char *restrict p_buffer, const char *restrict p_format, ...);
int snprintf(
    char *restrict p_buffer,
    size_t p_size,
    const char *restrict p_format,
    ...
);
int vsnprintf(
    char *p_buffer,
    size_t p_size,
    const char *p_format,
    va_list p_args
);
int vsprintf(char *p_buffer, const char *p_format, va_list p_args);
int printf(const char *p_format, ...);
int vprintf(const char *p_format, va_list p_args);

#endif
