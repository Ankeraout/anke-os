#ifndef __INCLUDE_DEBUG_H__
#define __INCLUDE_DEBUG_H__

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

typedef void tf_debugPrint(void *p_parameter, const char *p_value);

void debugInit(
    tf_debugPrint *p_writeFunc,
    void *p_parameter
);
void debug(const char *p_format, ...);

#endif
