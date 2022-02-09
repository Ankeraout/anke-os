// =============================================================================
// File inclusion
// =============================================================================
#include "klibc/klibc.h"

// =============================================================================
// Public functions definition
// =============================================================================
char *kstrcat(char *p_destination, const char *p_source) {
    size_t l_stringLength = kstrlen(p_destination);

    kstrcpy(p_destination + l_stringLength, p_source);

    return p_destination;
}

size_t kstrlen(const char *p_string) {
    size_t l_index = 0;

    while(p_string[l_index] != '\0') {
        l_index++;
    }

    return l_index;
}

char *kstrcpy(char *p_destination, const char *p_source) {
    size_t l_index = 0;

    do {
        p_destination[l_index] = p_source[l_index];
        l_index++;
    } while(p_source[l_index] != '\0');

    return p_destination;
}
