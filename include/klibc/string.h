#ifndef __INCLUDE_KLIBC_STRING_H__
#define __INCLUDE_KLIBC_STRING_H__

#include <stddef.h>
#include <stdint.h>

int kmemcmp(const void *p_ptr1, const void *p_ptr2, size_t p_size);
void *kmemcpy(void *p_dst, const void *p_src, size_t p_size);
void *kmemset(void *p_ptr, int p_value, size_t p_count);
int kstrcmp(const char *p_str1, const char *p_str2);
char *kstrcpy(char *p_dst, const char *p_src);
size_t kstrlen(const char *p_str);
int kstrncmp(const char *p_str1, const char *p_str2, size_t p_length);
char *kstrncpy(char *p_dst, const char *p_src, size_t p_length);
char *kstrrchr(const char *p_str, int p_character);
char *kstrchr(const char *p_str, int p_character);

#endif
