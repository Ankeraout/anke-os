#ifndef __INCLUDE_KLIBC_STDIO_H__
#define __INCLUDE_KLIBC_STDIO_H__

#include <stdarg.h>
#include <stddef.h>

int ksprintf(char *restrict p_buffer, const char *restrict p_format, ...);
int ksnprintf(
    char *restrict p_buffer,
    size_t p_size,
    const char *restrict p_format,
    ...
);
int kvsnprintf(
    char *p_buffer,
    size_t p_size,
    const char *p_format,
    va_list p_args
);
int kvsprintf(char *p_buffer, const char *p_format, va_list p_args);

#endif
