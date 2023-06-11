#if _KERNEL_TARGET_ARCH_X86_64 && _KERNEL_TARGET_BOOTLOADER_LIMINE

#include <stddef.h>

#include "kernel.h"
#include "kernel/arch/x86_64/inline.h"
#include "kernel/boot.h"
#include "klibc/debug.h"
#include "limine.h"

static struct limine_memmap_request s_memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct ts_kernelBootInfo l_kernelBootInfo;

static void initializeMemoryMap(struct ts_kernelBootInfo *p_bootInfo);
static void panic(const char *p_message);

void _start(void) {
    kernelDebug("AnkeOS kernel - x86_64-limine bootstrap\n");

    initializeMemoryMap(&l_kernelBootInfo);

    kernelMain(&l_kernelBootInfo);

    panic("kernelMain() returned");

    while(1);
}

static void initializeMemoryMap(struct ts_kernelBootInfo *p_bootInfo) {
    // If the memory map was not provided by Limine, panic.
    if(s_memmapRequest.response == NULL) {
        panic("Limine did not provide a memory map.");
    }

    // Compute the size of the memory map.
    size_t l_memoryMapSize = sizeof(struct limine_memmap_entry) * (s_memmapRequest.response->entry_count + 1);

    // Page align the size of the memory map
    l_memoryMapSize = (l_memoryMapSize + 0xfff) & ~(0xfff);

    // Find a free memory space to contain the memory map
    int l_freeEntryIndex = -1;

    for(uint64_t l_entryIndex = 0; l_entryIndex < s_memmapRequest.response->entry_count; l_entryIndex++) {
        if(s_memmapRequest.response->entries[l_entryIndex]->type == LIMINE_MEMMAP_USABLE) {
            if(s_memmapRequest.response->entries[l_entryIndex]->length >= l_memoryMapSize) {
                l_freeEntryIndex = (int)l_entryIndex;
            }
        }
    }

    if(l_freeEntryIndex == -1) {
        panic("Could not find a free memory area to copy the memory map.");
    }

    struct ts_kernelMemoryMapEntry *l_kernelMemoryMap = (struct ts_kernelMemoryMapEntry *)s_memmapRequest.response->entries[l_freeEntryIndex]->base;
    uint64_t l_entryWriteIndex = 0;

    // Copy the memory map
    for(uint64_t l_entryReadIndex = 0; l_entryReadIndex < s_memmapRequest.response->entry_count; l_entryReadIndex++) {
        if(l_entryReadIndex == (uint64_t)l_freeEntryIndex) {
            // Create a reclaimable entry
            l_kernelMemoryMap[l_entryWriteIndex].base = s_memmapRequest.response->entries[l_entryReadIndex]->base;
            l_kernelMemoryMap[l_entryWriteIndex].length = l_memoryMapSize;
            l_kernelMemoryMap[l_entryWriteIndex].type = E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE;
            l_entryWriteIndex++;

            l_kernelMemoryMap[l_entryWriteIndex].base = s_memmapRequest.response->entries[l_entryReadIndex]->base + l_memoryMapSize;
            l_kernelMemoryMap[l_entryWriteIndex].length = s_memmapRequest.response->entries[l_entryReadIndex]->length - l_memoryMapSize;
        } else {
            l_kernelMemoryMap[l_entryWriteIndex].base = s_memmapRequest.response->entries[l_entryReadIndex]->base;
            l_kernelMemoryMap[l_entryWriteIndex].length = s_memmapRequest.response->entries[l_entryReadIndex]->length;
        }

        switch(s_memmapRequest.response->entries[l_entryReadIndex]->type) {
            case LIMINE_MEMMAP_USABLE:
                l_kernelMemoryMap[l_entryWriteIndex].type = E_KERNELMEMORYMAPENTRYTYPE_FREE;
                break;

            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                l_kernelMemoryMap[l_entryWriteIndex].type = E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE;
                break;

            default:
                l_kernelMemoryMap[l_entryWriteIndex].type = E_KERNELMEMORYMAPENTRYTYPE_RESERVED;
                break;
        }

        l_entryWriteIndex++;
    }

    p_bootInfo->memoryMap = l_kernelMemoryMap;
    p_bootInfo->memoryMapEntryCount = s_memmapRequest.response->entry_count + 1;
}

static void panic(const char *p_message) {
    kernelDebug("Kernel panic: %s\n", p_message);

    while(1) {
        cli();
        hlt();
    }
}

#endif
