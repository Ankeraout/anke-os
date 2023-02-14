#include <stddef.h>

#include "klibc/stdlib.h"
#include "mm/pmm.h"

/* TODO: Keep track of the number of pages allocated at a time for kfree(). */
/* TODO: Map/unmap allocated pages in kernel space. */

void *kmalloc(size_t p_size) {
    return pmmAlloc(p_size);
}

void kfree(void *p_ptr) {
    pmmFree(p_ptr, 1);
}
