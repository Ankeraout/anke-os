#ifndef __INCLUDE_STRING_H__
#define __INCLUDE_STRING_H__

#include <stddef.h>

int memcmp(const void *p_ptr1, const void *p_ptr2, size_t p_size);
void *memcpy(void *p_dst, const void *p_src, size_t p_size);
void *memset(void *p_ptr, int p_value, size_t p_count);

#endif
