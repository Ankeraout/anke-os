#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/mm/pmm.h>

void *calloc(size_t p_size) {
    void *l_buffer = malloc(p_size);

    if(l_buffer == NULL) {
        return NULL;
    }

    memset(l_buffer, 0, p_size);

    return l_buffer;
}

void free(void *p_ptr) {
    M_UNUSED_PARAMETER(p_ptr);

    // Do nothing
}

void *malloc(size_t p_size) {
    M_UNUSED_PARAMETER(p_size);

    return NULL;
}
