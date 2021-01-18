#include <stddef.h>
#include <stdint.h>

void *memcpy(void *destination, const void *source, size_t size);
char *strcpy(char *dst, const char *src);
size_t strlen(const char *s);

void *memcpy(void *destination, const void *source, size_t size) {
    size_t *dst = (size_t *)destination;
    size_t *src = (size_t *)source;

    if((!((size_t)src & (SIZE_MAX - (sizeof(size_t) - 1)))) && (!((size_t)dst & (SIZE_MAX - (sizeof(size_t) - 1))))) {
        while(size >= sizeof(size_t)) {
            *dst++ = *src++;
            size -= sizeof(size_t);
        }
    }

    uint8_t *dst2 = (uint8_t *)dst;
    uint8_t *src2 = (uint8_t *)src;

    while(size--) {
        *dst2++ = *src2++;
    }

    return destination;
}

char *strcpy(char *dst, const char *src) {
    size_t i = 0;

    do {
        dst[i] = src[i];
        i++;
    } while(src[i]);

    return dst;
}

size_t strlen(const char *s) {
    size_t n = 0;

    while(s[n]) {
        n++;
    }

    return n;
}
