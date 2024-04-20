#ifndef __INCLUDE_STDIO_H__
#define __INCLUDE_STDIO_H__

#include <stdarg.h>
#include <stddef.h>

#define C_PRINTF_BUFFER_LENGTH 512

int printf(const char *restrict p_format, ...);
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
int puts(const char *p_string);
int putchar(int p_character);

#endif
