#include <stddef.h>

#include <kernel/common.h>

void vmmInit(void) {

}

void *vmmAlloc(size_t p_size) {
    M_UNUSED_PARAMETER(p_size);

    return NULL;
}

void vmmFree(void *p_ptr, size_t p_size) {
    M_UNUSED_PARAMETER(p_ptr);
    M_UNUSED_PARAMETER(p_size);
}

void *vmmMap(void *p_pptr, void *p_vptr, size_t p_size, int p_prot) {
    M_UNUSED_PARAMETER(p_pptr);
    M_UNUSED_PARAMETER(p_vptr);
    M_UNUSED_PARAMETER(p_size);
    M_UNUSED_PARAMETER(p_prot);

    return NULL;
}

void vmmUnmap(void *p_vptr, size_t p_size) {
    M_UNUSED_PARAMETER(p_vptr);
    M_UNUSED_PARAMETER(p_size);
}

void *vmmProtect(void *p_ptr, size_t p_size, int p_prot) {
    M_UNUSED_PARAMETER(p_ptr);
    M_UNUSED_PARAMETER(p_size);
    M_UNUSED_PARAMETER(p_prot);

    return NULL;
}
