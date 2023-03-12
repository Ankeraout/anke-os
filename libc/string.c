#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int memcmp(const void *p_ptr1, const void *p_ptr2, size_t p_size) {
    for(size_t l_index = 0; l_index < p_size; l_index++) {
        const uint8_t l_char1 = ((const uint8_t *)p_ptr1)[l_index];
        const uint8_t l_char2 = ((const uint8_t *)p_ptr2)[l_index];

        if(l_char1 != l_char2) {
            if(l_char1 < l_char2) {
                return -1;
            } else if(l_char1 > l_char2) {
                return 1;
            }
        }
    }

    return 0;
}

void *memcpy(void *p_dst, const void *p_src, size_t p_size) {
    const uint8_t *l_arraySource = (const uint8_t *)p_src;
    uint8_t *l_arrayDestination = (uint8_t *)p_dst;

    for(size_t l_index = 0; l_index < p_size; l_index++) {
        l_arrayDestination[l_index] = l_arraySource[l_index];
    }

    return p_dst;
}

void *memset(void *p_ptr, int p_value, size_t p_count) {
    uint8_t *l_array = (uint8_t *)p_ptr;

    for(size_t l_index = 0; l_index < p_count; l_index++) {
        l_array[l_index] = p_value;
    }

    return p_ptr;
}

int strcmp(const char *p_str1, const char *p_str2) {
    size_t l_index = 0;

    while((p_str1[l_index] == p_str2[l_index]) && (p_str1[l_index] != '\0')) {
        l_index++;
    }

    if(p_str1[l_index] == p_str2[l_index]) {
        return 0;
    } else if(p_str1[l_index] < p_str2[l_index]) {
        return -1;
    } else {
        return 1;
    }
}

char *strcpy(char *p_dst, const char *p_src) {
    size_t l_index = 0;

    do {
        p_dst[l_index] = p_src[l_index];
        l_index++;
    } while(p_src[l_index] != '\0');

    return p_dst;
}

char *strdup(const char *p_src) {
    size_t l_length = strlen(p_src) + 1;
    char *l_copy = malloc(l_length);

    if(l_copy == NULL) {
        return NULL;
    }

    memcpy(l_copy, p_src, l_length);

    return l_copy;
}

size_t strlen(const char *p_str) {
    size_t l_length = 0;

    while(p_str[l_length] != '\0') {
        l_length++;
    }

    return l_length;
}


int strncmp(const char *p_str1, const char *p_str2, size_t p_length) {
    size_t l_index = 0;

    while(
        (p_str1[l_index] == p_str2[l_index])
        && (p_str1[l_index] != '\0')
        && (l_index < p_length)
    ) {
        l_index++;
    }

    if(p_str1[l_index] == p_str2[l_index]) {
        return 0;
    } else if(p_str1[l_index] < p_str2[l_index]) {
        return -1;
    } else {
        return 1;
    }
}

char *strncpy(char *p_dst, const char *p_src, size_t p_length) {
    size_t l_index = 0;

    do {
        p_dst[l_index] = p_src[l_index];
        l_index++;
    } while(
        (p_src[l_index] != '\0')
        && (l_index < p_length)
    );

    return p_dst;
}

char *strndup(const char *p_src, size_t p_length) {
    size_t l_length = strlen(p_src);

    if(l_length > p_length) {
        l_length = p_length;
    }

    char *l_copy = malloc(l_length + 1);

    memcpy(l_copy, p_src, l_length);

    l_copy[l_length] = '\0';

    return l_copy;
}

char *strrchr(const char *p_str, int p_character) {
    const char *l_lastOccurrence = NULL;

    while(*p_str != '\0') {
        if(*p_str == p_character) {
            l_lastOccurrence = p_str;
        }

        p_str++;
    }

    return (char *)l_lastOccurrence;
}
