#ifndef __KERNEL_LIBK_STRING_H__
#define __KERNEL_LIBK_STRING_H__

#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t size);
void *memset(void *dst, int c, size_t n);
char *strcpy(char *dst, const char *src);
size_t strlen(const char *s);

#endif
