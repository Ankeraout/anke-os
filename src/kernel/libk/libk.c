#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libk/libk.h"

#define DEF_MEMSET(type) \
    for(size_t i = 0; i < n / sizeof(type); i++) { \
        ((type *)str)[i] = value; \
    }

#define DEF_MEMCPY(type) \
    for(size_t i = 0; i < n / sizeof(type); i++) { \
        ((type *)dest)[i] = ((type *)src)[i]; \
    }

#define DEF_HEX(n) \
    char *hex##n(uint##n##_t value, char *str) { \
        for(int i = 0; i < (n >> 2); i++) { \
            str[i] = "0123456789abcdef"[value & 0x0f]; \
            value >>= 4; \
        } \
        \
        str[n >> 2] = '\0'; \
        \
        return strrev(str); \
    }

void *memset(void *str, int c, size_t n) {
    size_t address = (size_t)str;

    if(!(address & 0x00000003) && !(n & 0x00000003)) {
        uint32_t value = (uint32_t)c;

        value |= value << 8;
        value |= value << 16;

        DEF_MEMSET(uint32_t);
    } else if(!(address & 0x00000001) && !(n & 0x00000001)) {
        uint16_t value = (uint16_t)c;
        
        value |= value << 8;

        DEF_MEMSET(uint32_t);
    } else {
        uint8_t value = (uint8_t)c;

        DEF_MEMSET(uint32_t);
    }

    return str;
}

void *memcpy(void *dest, const void *src, size_t n) {
    size_t address = (size_t)dest;

    if(!(address & 0x00000003) && !(n & 0x00000003)) {
        DEF_MEMCPY(uint32_t);
    } else if(!(address & 0x00000001) && !(n & 0x00000001)) {
        DEF_MEMCPY(uint16_t);
    } else {
        DEF_MEMCPY(uint8_t);
    }

    return dest;
}

size_t strlen(const char *str) {
    size_t stringLength = 0;

    while(str[stringLength]) {
        stringLength++;
    }
    
    return stringLength;
}

char *strrev(char *str) {
    size_t i = 0;
    size_t j = strlen(str) - 1;

    while(j > i) {
        char tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;

        i++;
        j--;
    }

    return str;
}

char *itoa(int value, char *str, int base) {
    const char *basebuf = "0123456789abcdefghijklmnopqrstuvwxyz";

    bool minusSign = false;
    size_t uvalue = value;

    if((base < 2) || (base > 36)) {
        return str;
    }

    size_t index = 0;

    if((base == 10) && (value < 0)) {
        minusSign = true;
        uvalue = -value;
    }

    do {
        str[index++] = basebuf[uvalue % base];
        uvalue /= base;
    } while(uvalue);

    if(minusSign) {
        str[index++] = '-';
    }

    str[index] = '\0';

    return strrev(str);
}

DEF_HEX(8);
DEF_HEX(16);
DEF_HEX(32);
DEF_HEX(64);