#ifndef __INCLUDE_DEBUG_H__
#define __INCLUDE_DEBUG_H__

#include "dev/debugcon.h"

typedef void (*t_debugWriteFunc)(void *p_parameter, uint8_t p_value);

void debugInit(t_debugWriteFunc p_writeFunc, void *p_parameter);
void debugPrint(const char *p_string);
void debugWrite(const void *p_buffer, size_t p_size);
void debugPrintPointer(const void *p_ptr);

#endif
