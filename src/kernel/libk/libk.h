#ifndef __LIBK_H__
#define __LIBK_H__

#include <stddef.h>
#include <stdint.h>

#define UNUSED_PARAMETER(x) (void)x

#define DECL_HEX(n) \
    char *hex##n(uint##n##_t value, char *str)

void *memset(void *str, int c, size_t n);
void *memcpy(void *dest, const void* src, size_t n);
size_t strlen(const char *str);
char *strrev(char *str);
char *itoa(int value, char *str, int base);
void *malloc(size_t size);
void free(const void *addr);

DECL_HEX(8);
DECL_HEX(16);
DECL_HEX(32);
DECL_HEX(64);

#endif
