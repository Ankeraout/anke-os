#ifndef __KERNEL_LIBK_STRING_H__
#define __KERNEL_LIBK_STRING_H__

#include <stddef.h>

int memcmp(const void *ptr1, const void *ptr2, size_t size);
void *memcpy(void *destination, const void *source, size_t size);
void *memset(void *dst, int c, size_t n);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dst, const char *src);
size_t strlen(const char *s);
char *strrev(char *s);

#endif
