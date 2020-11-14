#ifndef __KERNEL_LIBK_LIBK_HPP__
#define __KERNEL_LIBK_LIBK_HPP__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UNUSED_PARAMETER(x) (void)x

#define DECL_HEX(n) \
    char *hex##n(uint##n##_t value, char *str)

namespace std {
    void free(const void *ptr);
    char *itoa(int value, char *str, int base);
    void *malloc(size_t size, bool kernel);
    int memcmp(const void *mb1, const void *mb2, size_t n);
    void *memcpy(void *dst, const void* src, size_t n);
    void *memset(void *str, int c, size_t n);
    size_t strlen(const char *str);
    char *strncpy(char *dst, const char *src, size_t num);
    char *strrev(char *str);
    int strncmp(const char *str1, const char *str2, size_t num);

    DECL_HEX(8);
    DECL_HEX(16);
    DECL_HEX(32);
    DECL_HEX(64);
}

#endif
