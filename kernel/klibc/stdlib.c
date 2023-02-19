#include <stddef.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/mm/pmm.h>

/* TODO: Keep track of the number of pages allocated at a time for kfree(). */
/* TODO: Map/unmap allocated pages in kernel space. */

void *kcalloc(size_t p_size) {
    void *l_buffer = kmalloc(p_size);

    if(l_buffer == NULL) {
        return NULL;
    }

    memset(l_buffer, 0, p_size);

    return l_buffer;
}

void kfree(void *p_ptr) {
    pmmFree(p_ptr, 1);
}

void *kmalloc(size_t p_size) {
    return pmmAlloc(p_size);
}

void *krealloc(void *p_ptr, size_t p_size) {
    M_UNUSED_PARAMETER(p_size);

    /*TODO*/

    return p_ptr;
}
