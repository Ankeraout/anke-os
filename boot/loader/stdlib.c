#include "boot/loader/mm.h"
#include "boot/loader/stdlib.h"

struct ts_allocStructure {
    size_t size;
    int data[];
};

void *malloc(size_t p_size) {
    const size_t l_finalSize = p_size + sizeof(void *);

    struct ts_allocStructure *l_result = mm_alloc(l_finalSize);

    if(l_result == NULL) {
        return NULL;
    }
    
    l_result->size = l_finalSize;

    return l_result->data;
}

void free(void *p_ptr) {
    const uintptr_t l_address = (const uintptr_t)p_ptr;
    const uintptr_t l_structureAddress = l_address - sizeof(void *);
    
    struct ts_allocStructure *l_structure =
        (struct ts_allocStructure *)l_structureAddress;

    mm_free(l_structure, l_structure->size);
}