#include <stddef.h>
#include <stdint.h>

#include "kernel/boot.h"
#include "kernel/mm/pmm.h"
#include "klibc/debug.h"
#include "klibc/linkedlist.h"

#define C_PMM_BUDDY_COUNT 8

#ifdef C_PMM_DEBUG
#define pmmDebug kernelDebug
#else
#define pmmDebug pmmDebugNull
#endif

static struct ts_linkedListNode *s_pmmBuddies[C_PMM_BUDDY_COUNT];

static void *pmmAllocBuddy(int l_buddyIndex);
static int pmmDebugNull();

int pmmInit(const struct ts_kernelBootInfo *p_bootInfo) {
    for(int l_index = 0; l_index < C_PMM_BUDDY_COUNT; l_index++) {
        s_pmmBuddies[l_index] = NULL;
    }

    for(size_t l_entryIndex = 0; l_entryIndex < p_bootInfo->memoryMapEntryCount; l_entryIndex++) {
        const struct ts_kernelMemoryMapEntry *l_memoryMapEntry = &p_bootInfo->memoryMap[l_entryIndex];

        if(l_memoryMapEntry->type == E_KERNELMEMORYMAPENTRYTYPE_FREE) {
            pmmFree((void *)l_memoryMapEntry->base, l_memoryMapEntry->length);
        }
    }

    // TODO: reclaimable memory areas

    return 0;
}

void *pmmAlloc(size_t p_size) {
    const size_t l_sizePages = (p_size + 0xfff) >> 12;
    const size_t l_biggestBuddySize = 4096 << (C_PMM_BUDDY_COUNT - 1);

    if(p_size > l_biggestBuddySize) {
        return NULL;
    }

    size_t l_buddyIndex = C_PMM_BUDDY_COUNT - 1;
    size_t l_buddySizePages = 1 << l_buddyIndex;
    size_t l_buddySizeBytes = l_buddySizePages << 12;

    while(l_buddySizeBytes >= (p_size << 1)) {
        l_buddyIndex--;
        l_buddySizePages >>= 1;
        l_buddySizeBytes >>= 1;
    }

    if(l_buddyIndex >= C_PMM_BUDDY_COUNT) {
        l_buddyIndex = 0;
        l_buddySizeBytes = 4096;
        l_buddySizePages = 1;
    }

    pmmDebug("pmmAlloc: allocating %lu-byte buddy.\n", l_buddySizeBytes);

    void *l_ptr = pmmAllocBuddy(l_buddyIndex);

    if(l_ptr == NULL) {
        pmmDebug("pmmAlloc: allocation failed.\n");
        return NULL;
    }

    const size_t l_address = (size_t)l_ptr;

    pmmDebug("pmmAlloc: obtained 0x%016lx.\n", l_address);

    const size_t l_pagesToFree = l_sizePages - l_buddySizePages;
    const size_t l_bytesToFree = l_pagesToFree << 12;

    if(l_bytesToFree > 0) {
        pmmDebug("pmmAlloc: Freeing %lu bytes.\n", l_bytesToFree);
        pmmFree((void *)(l_address + p_size), l_bytesToFree);
    }

    return l_ptr;
}

void pmmFree(void *p_ptr, size_t p_size) {
    size_t l_ptr = (size_t)p_ptr;
    size_t l_nbPages = p_size >> 12;
    size_t l_buddyIndex = C_PMM_BUDDY_COUNT - 1;
    size_t l_buddySizePages = 1 << l_buddyIndex;
    size_t l_buddySizeBytes = (4096 << l_buddyIndex);

    while(l_nbPages > 0) {
        if(l_nbPages >= l_buddySizePages) {
            struct ts_linkedListNode *l_linkedListNode = (struct ts_linkedListNode *)l_ptr;

            l_linkedListNode->data = (void *)l_ptr;
            l_linkedListNode->next = s_pmmBuddies[l_buddyIndex];

            s_pmmBuddies[l_buddyIndex] = l_linkedListNode;

            pmmDebug("pmmFree: Freed %lu-byte buddy at 0x%016lx.\n", l_buddySizeBytes, l_ptr);

            l_nbPages -= l_buddySizePages;
            l_ptr += l_buddySizeBytes;
        } else {
            l_buddyIndex--;
            l_buddySizePages >>= 1;
            l_buddySizeBytes >>= 1;
        }
    }
}

static void *pmmAllocBuddy(int l_buddyIndex) {
    pmmDebug("pmmAllocBuddy: requested %lu bytes.\n", 4096 << l_buddyIndex);

    if(s_pmmBuddies[l_buddyIndex] != NULL) {
        pmmDebug("pmmAllocBuddy: found buddy of this size.\n");
        struct ts_linkedListNode *l_buddy = s_pmmBuddies[l_buddyIndex];
        s_pmmBuddies[l_buddyIndex] = l_buddy->next;
        return l_buddy;
    } else if(l_buddyIndex >= (C_PMM_BUDDY_COUNT - 1)) {
        pmmDebug("pmmAllocBuddy: out of memory.\n");
        return NULL;
    } else {
        pmmDebug("pmmAllocBuddy: splitting larger block.\n");
        size_t l_buddySizeBytes = (4096 << l_buddyIndex);
        void *l_buddy = pmmAllocBuddy(l_buddyIndex + 1);

        if(l_buddy == NULL) {
            return NULL;
        }

        size_t l_buddyAddress = (size_t)l_buddy;

        // Free the second half of the buddy
        pmmFree((void *)(l_buddyAddress + l_buddySizeBytes), l_buddySizeBytes);

        return (void *)l_buddyAddress;
    }
}

static int pmmDebugNull() {
    return 0;
}
