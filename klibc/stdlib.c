#include <stddef.h>

#include "kernel/mm/pmm.h"
#include "klibc/stdlib.h"

void *kmalloc(size_t p_size) {
    const size_t l_finalSize = p_size + sizeof(size_t);

    void *l_ptr = pmmAlloc(l_finalSize);

    if(l_ptr == NULL) {
        return NULL;
    }

    *(size_t *)l_ptr = l_finalSize;

    return l_ptr;
}

void kfree(void *p_ptr) {
    const size_t l_finalSize = *(size_t *)p_ptr;

    pmmFree(p_ptr, l_finalSize);
}
