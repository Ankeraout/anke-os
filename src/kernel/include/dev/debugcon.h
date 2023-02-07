#ifndef __INCLUDE_DEV_DEBUGCON_H__
#define __INCLUDE_DEV_DEBUGCON_H__

#include <stddef.h>
#include <stdint.h>

void debugconPutc(int p_character);
void debugconPuts(const char *p_string);
void debugconWrite(const void *p_buffer, size_t p_size);

#endif
