#include <stddef.h>
#include <stdint.h>

int memcmp(const void *ptr1, const void *ptr2, size_t size);
void *memcpy(void *destination, const void *source, size_t size);
void *memset(void *dst, int c, size_t n);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
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

void *memset(void *destination, int c, size_t n) {
    size_t *dst = (size_t *)destination;

    if(!((size_t)dst & (SIZE_MAX - (sizeof(size_t) - 1)))) {
        size_t v = 0;

        for(size_t i = 0; i < sizeof(size_t); i++) {
            v <<= 8;
            v |= c;
        }

        while(n >= sizeof(size_t)) {
            *dst++ = v;
            n -= sizeof(size_t);
        }
    }

    uint8_t *dst2 = (uint8_t *)dst;

    while(n--) {
        *dst2++ = c;
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

int strcmp(const char *s1, const char *s2) {
    while((*s1 == *s2) && *s1) {
        s1++;
        s2++;
    }

    if(s1 < s2) {
        return -1;
    } else if(s1 > s2) {
        return 1;
    } else {
        return 0;
    }
}

int memcmp(const void *ptr1, const void *ptr2, size_t size) {
    const uint8_t *buffer1 = (const uint8_t *)ptr1;
    const uint8_t *buffer2 = (const uint8_t *)ptr2;
    
    for(size_t i = 0; i < size; i++) {
        if(buffer1[i] < buffer2[i]) {
            return -1;
        } else if(buffer1[i] > buffer2[i]) {
            return 1;
        }
    }

    return 0;
}

char *strrev(char *s) {
    size_t stringLength = strlen(s);

    size_t i = 0;
    size_t j = stringLength - 1;

    while(i < j) {
        char exchange = s[i];
        s[i] = s[j];
        s[j] = exchange;

        i++;
        j--;
    }

    return s;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t i = 0;

    while(src[i] && i < n) {
        dst[i] = src[i];
        i++;
    }

    dst[i] = '\0';

    return dst;
}
