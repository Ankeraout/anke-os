#include "stdlib.h"
#include "mm/pmm.h"
#include "mm/vmm.h"

void *malloc(size_t p_size) {
    struct ts_vmm_context *l_kernelContext = vmm_getKernelContext();

    void *l_virtualPtr = vmm_alloc(
        l_kernelContext,
        p_size + sizeof(void *),
        C_VMM_ALLOC_FLAG_KERNEL
    );

    if(l_virtualPtr == NULL) {
        return NULL;
    }

    void *l_physicalPtr = pmm_alloc(p_size + sizeof(void *));

    if(l_physicalPtr == NULL) {
        vmm_free(l_kernelContext, l_virtualPtr, p_size + sizeof(void *));
        return NULL;
    }

    if(
        vmm_map(
            l_kernelContext,
            l_virtualPtr,
            l_physicalPtr,
            p_size + sizeof(void *),
            C_VMM_PROT_KERNEL | C_VMM_PROT_READ_WRITE
        ) != 0
    ) {
        pmm_free(l_physicalPtr, p_size + sizeof(void *));
        vmm_free(l_kernelContext, l_virtualPtr, p_size + sizeof(void *));
        return NULL;
    }

    *(size_t *)l_virtualPtr = p_size;

    return (void *)((uintptr_t)l_virtualPtr + sizeof(void *));
}

void free(void *p_ptr) {
    size_t l_size = *(size_t *)p_ptr;

    pmm_free(vmm_getPhysicalAddress(p_ptr), l_size + sizeof(void *));
    vmm_free(vmm_getKernelContext(), p_ptr, l_size + sizeof(void *));
}
