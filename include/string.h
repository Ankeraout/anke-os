#ifndef __INCLUDE_STRING_H__
#define __INCLUDE_STRING_H__

#include <stddef.h>

int memcmp(const void *p_ptr1, const void *p_ptr2, size_t p_size);
void *memcpy(void *p_dst, const void *p_src, size_t p_size);
void *memset(void *p_ptr, int p_value, size_t p_count);
int strcmp(const char *p_str1, const char *p_str2);
char *strcpy(char *p_dst, const char *p_src);
char *strdup(const char *p_src);
size_t strlen(const char *p_str);
int strncmp(const char *p_str1, const char *p_str2, size_t p_length);
char *strncpy(char *p_dst, const char *p_src, size_t p_length);
char *strndup(const char *p_src, size_t p_length);

#endif
