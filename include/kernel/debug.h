#ifndef __INCLUDE_DEBUG_H__
#define __INCLUDE_DEBUG_H__

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

void debugInit(
    void (*p_writeFunc)(void *p_parameter, char p_value),
    void *p_parameter
);
void debug(const char *p_format, ...);

#endif
