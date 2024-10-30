#include <stdbool.h>

#include "boot/loader/mm.h"

struct ts_memoryDescriptor {
    struct ts_memoryDescriptor *next;
    size_t size;
};

struct ts_memoryDescriptor *s_descriptorList = NULL;

int mm_init(const struct ts_bootInfoStructure *p_bootInfoStructure) {
    const struct ts_memoryMapEntry *l_memoryMap =
        (const struct ts_memoryMapEntry *)
            p_bootInfoStructure->m_memoryMapAddress;
    const unsigned int l_memoryMapEntries =
        p_bootInfoStructure->m_memoryMapSize / sizeof(struct ts_memoryMapEntry);
    
    for(unsigned int l_index = 0; l_index < l_memoryMapEntries; l_index++) {
        const bool l_tooLow = l_memoryMap[l_index].m_base < 0x100000ULL;
        const bool l_tooHigh = l_memoryMap[l_index].m_base >= 0x100000000ULL;
        const uint64_t l_end =
            l_memoryMap[l_index].m_base + l_memoryMap[l_index].m_size;
        const bool l_endsHigh = l_end >= 0x100000000ULL;
        const uint64_t l_fixedSize =
            l_tooHigh
                ? 0ULL
                : (
                    l_endsHigh
                        ? l_memoryMap[l_index].m_size - (l_end - 0x100000000ULL)
                        : l_memoryMap[l_index].m_size
                );
        const bool l_notFree = l_memoryMap[l_index].m_type != 1;
        const bool l_tooSmall =
            l_fixedSize < sizeof(struct ts_memoryDescriptor);

        if(l_tooLow || l_notFree || l_tooSmall) {
            continue;
        }

        struct ts_memoryDescriptor *l_descriptor =
            (struct ts_memoryDescriptor *)(uint32_t)l_memoryMap[l_index].m_base;

        l_descriptor->next = s_descriptorList;
        l_descriptor->size = l_fixedSize;

        s_descriptorList = l_descriptor;
    }

    return 0;
}

void *mm_alloc(size_t p_size) {
    struct ts_memoryDescriptor *l_currentDescriptor = s_descriptorList;
    struct ts_memoryDescriptor *l_previousDescriptor = NULL;

    const size_t l_roundedSize =
        (p_size % sizeof(size_t)) != 0ULL
        ? p_size + sizeof(size_t) - (p_size % sizeof(size_t))
        : p_size;
    const size_t l_descriptorSize =
        l_roundedSize + sizeof(struct ts_memoryDescriptor);

    while(l_currentDescriptor != NULL) {
        if(l_currentDescriptor->size == l_roundedSize) {
            if(l_previousDescriptor == NULL) {
                s_descriptorList = l_currentDescriptor->next;
            } else {
                l_previousDescriptor->next = l_currentDescriptor->next;
            }

            return l_currentDescriptor;
        } else if(l_currentDescriptor->size >= l_descriptorSize) {
            const uintptr_t l_descriptorPointer =
                (const uintptr_t)l_currentDescriptor;
            const uintptr_t l_newDescriptorPointer =
                l_descriptorPointer + l_roundedSize;

            struct ts_memoryDescriptor *l_newDescriptor =
                (struct ts_memoryDescriptor *)l_newDescriptorPointer;

            l_newDescriptor->next = l_currentDescriptor->next;
            l_newDescriptor->size = l_currentDescriptor->size - l_roundedSize;

            if(l_previousDescriptor == NULL) {
                s_descriptorList = l_newDescriptor;
            } else {
                l_previousDescriptor->next = l_newDescriptor;
            }

            return l_currentDescriptor;
        }

        l_previousDescriptor = l_currentDescriptor;
        l_currentDescriptor = l_currentDescriptor->next;
    }

    return NULL;
}

void mm_free(void *p_ptr, size_t p_size) {
    const size_t l_roundedSize =
        (p_size % sizeof(size_t)) != 0ULL
        ? p_size + sizeof(size_t) - (p_size % sizeof(size_t))
        : p_size;

    if(l_roundedSize < sizeof(struct ts_memoryDescriptor)) {
        return;
    }

    struct ts_memoryDescriptor *l_descriptor =
        (struct ts_memoryDescriptor *)p_ptr;
    
    l_descriptor->next = s_descriptorList;
    l_descriptor->size = p_size;

    s_descriptorList = l_descriptor;
}
