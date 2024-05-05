#include "kernel/kmalloc.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"

void *kmalloc(size_t p_size, int p_flags) {
    void *l_virtualPtr = vmmAlloc(
        vmmGetKernelContext(),
        p_size + 8UL,
        C_VMM_ALLOC_FLAG_KERNEL
    );

    if(l_virtualPtr == NULL) {
        return NULL;
    }

    void *l_physicalPtr = pmmAlloc(p_size + 8UL);

    if(l_physicalPtr == NULL) {
        vmmFree(vmmGetKernelContext(), l_virtualPtr, p_size + 8UL);
        return NULL;
    }

    if(
        vmmMap(
            vmmGetKernelContext(),
            l_virtualPtr,
            l_physicalPtr,
            p_size + 8UL,
            C_VMM_PROT_KERNEL | C_VMM_PROT_READ_WRITE
        ) != 0
    ) {
        pmmFree(l_physicalPtr, p_size + 8UL);
        vmmFree(vmmGetKernelContext(), l_virtualPtr, p_size + 8UL);
        return NULL;
    }

    *(size_t *)l_virtualPtr = p_size;

    return (void *)((uintptr_t)l_virtualPtr + 8UL);
}

void kfree(void *p_ptr) {
    size_t l_size = *(size_t *)p_ptr;

    pmmFree(vmmGetPhysicalAddress(p_ptr), l_size + 8UL);
    vmmFree(vmmGetKernelContext(), p_ptr, l_size + 8UL);
}
