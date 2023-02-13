#include <stddef.h>
#include <stdint.h>

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

void *memset(void *p_ptr, int p_value, size_t p_count) {
    uint8_t *l_array = (uint8_t *)p_ptr;

    for(size_t l_index = 0; l_index < p_count; l_index++) {
        l_array[l_index] = p_value;
    }

    return p_ptr;
}
