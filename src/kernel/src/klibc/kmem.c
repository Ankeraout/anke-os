// =============================================================================
// File inclusion
// =============================================================================
#include <stddef.h>
#include <stdint.h>

#include "klibc/klibc.h"

// =============================================================================
// Public functions definition
// =============================================================================
void *kmemcpy(void *p_destination, const void *p_source, size_t p_size) {
    uint8_t *l_destination = (uint8_t *)p_destination;
    const uint8_t *l_source = (const uint8_t *)p_source;

    for(size_t l_index = 0; l_index < p_size; l_index++) {
        l_destination[l_index] = l_source[l_index];
    }

    return p_destination;
}

void *kmemmove(void *p_destination, const void *p_source, size_t p_size) {
    size_t l_sourceStart = (size_t)p_source;
    size_t l_sourceEnd = l_sourceStart + p_size - 1;
    size_t l_destinationStart = (size_t)p_destination;
    size_t l_destinationEnd = l_destinationStart + p_size - 1;

    if(
        (l_sourceEnd < l_destinationStart)
        || (l_destinationEnd < l_sourceStart)
        || (l_destinationStart < l_sourceStart)
    ) {
        return kmemcpy(p_destination, p_source, p_size);
    } else if(l_sourceStart < l_destinationStart) {
        uint8_t *l_destination = (uint8_t *)p_destination;
        const uint8_t *l_source = (const uint8_t *)p_source;

        // l_index becomes > p_size when underflowing
        for(size_t l_index = p_size - 1; l_index < p_size; l_index--) {
            l_destination[l_index] = l_source[l_index];
        }
    }

    return p_destination;
}

void *kmemset(void *p_buffer, int p_value, size_t p_size) {
    uint8_t *l_buffer = (uint8_t *)p_buffer;

    for(size_t l_index = 0; l_index < p_size; l_index++) {
        *l_buffer++ = (uint8_t)p_value;
    }

    return p_buffer;
}
