#include "stdlib.h"
#include "mm/pmm.h"
#include "mm/vmm.h"

void *malloc(size_t p_size) {
    void *l_virtualPtr = vmmAlloc(
        vmmGetKernelContext(),
        p_size + sizeof(void *),
        C_VMM_ALLOC_FLAG_KERNEL
    );

    if(l_virtualPtr == NULL) {
        return NULL;
    }

    void *l_physicalPtr = pmmAlloc(p_size + sizeof(void *));

    if(l_physicalPtr == NULL) {
        vmmFree(vmmGetKernelContext(), l_virtualPtr, p_size + sizeof(void *));
        return NULL;
    }

    if(
        vmmMap(
            vmmGetKernelContext(),
            l_virtualPtr,
            l_physicalPtr,
            p_size + sizeof(void *),
            C_VMM_PROT_KERNEL | C_VMM_PROT_READ_WRITE
        ) != 0
    ) {
        pmmFree(l_physicalPtr, p_size + sizeof(void *));
        vmmFree(vmmGetKernelContext(), l_virtualPtr, p_size + sizeof(void *));
        return NULL;
    }

    *(size_t *)l_virtualPtr = p_size;

    return (void *)((uintptr_t)l_virtualPtr + sizeof(void *));
}

void free(void *p_ptr) {
    size_t l_size = *(size_t *)p_ptr;

    pmmFree(vmmGetPhysicalAddress(p_ptr), l_size + sizeof(void *));
    vmmFree(vmmGetKernelContext(), p_ptr, l_size + sizeof(void *));
}
