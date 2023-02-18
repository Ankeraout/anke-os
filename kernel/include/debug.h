#ifndef __INCLUDE_DEBUG_H__
#define __INCLUDE_DEBUG_H__

#include <stddef.h>
#include <stdint.h>

typedef void tf_debugWriteFunc(void *p_parameter, char p_value);

void debugInit(tf_debugWriteFunc *p_writeFunc, void *p_parameter);
void debugPrint(const char *p_string);
void debugPrintHex8(uint8_t p_value);
void debugPrintHex16(uint16_t p_value);
void debugPrintHex32(uint32_t p_value);
void debugPrintHex64(uint64_t p_value);
void debugWrite(const void *p_buffer, size_t p_size);
void debugPrintPointer(const void *p_ptr);

#endif
