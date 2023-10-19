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

    return (void *)((size_t)l_ptr + sizeof(size_t));
}

void kfree(void *p_ptr) {
    void *l_finalPtr = (void *)((size_t)p_ptr - sizeof(size_t));
    const size_t l_finalSize = *(size_t *)l_finalPtr;

    pmmFree(l_finalPtr, l_finalSize);
}
