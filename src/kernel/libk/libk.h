#ifndef __LIBK_H__
#define __LIBK_H__

#include <stddef.h>

#define UNUSED_PARAMETER(x) (void)x

void *memset(void *str, int c, size_t n);
void *memcpy(void *dest, const void* src, size_t n);
size_t strlen(const char *str);
char *strrev(char *str);
char *itoa(int value, char *str, int base);

#endif
