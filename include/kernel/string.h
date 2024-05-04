#ifndef __INCLUDE_KERNEL_STRING_H__
#define __INCLUDE_KERNEL_STRING_H__

#include <stddef.h>
#include <stdint.h>

int memcmp(const void *p_ptr1, const void *p_ptr2, size_t p_size);
void *memcpy(void *p_dst, const void *p_src, size_t p_size);
void *memset(void *p_ptr, int p_value, size_t p_count);
int strcmp(const char *p_str1, const char *p_str2);
char *strcpy(char *p_dst, const char *p_src);
size_t strlen(const char *p_str);
int strncmp(const char *p_str1, const char *p_str2, size_t p_length);
char *strncpy(char *p_dst, const char *p_src, size_t p_length);
char *strrchr(const char *p_str, int p_character);
char *strchr(const char *p_str, int p_character);

#endif