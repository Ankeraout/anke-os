#include <stddef.h>

#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "klibc/stdlib.h"
#include "klibc/debug.h"

void *kmalloc(size_t p_size) {
    size_t l_size = mmRoundUpPage(p_size + 16);

    void *l_vptr = vmmAlloc(
        vmmGetKernelContext(),
        l_size,
        C_VMM_ALLOC_FLAG_KERNEL
    );

    if(l_vptr == NULL) {
        return NULL;
    }

    void *l_pptr = pmmAlloc(l_size);

    if(l_pptr == NULL) {
        vmmFree(vmmGetKernelContext(), l_vptr, l_size);
        return NULL;
    }

    if(
        vmmMap(
            vmmGetKernelContext(),
            l_vptr,
            l_pptr,
            l_size,
            C_VMM_PROT_KERNEL
            | C_VMM_PROT_READ_WRITE
            | C_VMM_PROT_EXEC
        ) != 0
    ) {
        vmmFree(vmmGetKernelContext(), l_vptr, l_size);
        pmmFree(l_pptr, l_size);
        return NULL;
    }

    uint64_t *l_header = l_vptr;

    l_header[0] = (uint64_t)l_pptr;
    l_header[1] = (uint64_t)p_size;

    return &l_header[2];
}

void kfree(void *p_ptr) {
    uint64_t *l_header = p_ptr;

    pmmFree((void *)l_header[-2], l_header[-1] + 16);
    vmmUnmap(vmmGetKernelContext(), p_ptr, l_header[-1] + 16);
    vmmFree(vmmGetKernelContext(), p_ptr, l_header[-1] + 16);
}
