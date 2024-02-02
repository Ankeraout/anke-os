#if _KERNEL_TARGET_ARCH_X86_64 && _KERNEL_TARGET_BOOTLOADER_LIMINE

#include <stddef.h>

#include "kernel.h"
#include "kernel/arch/x86_64/inline.h"
#include "kernel/boot.h"
#include "klibc/debug.h"
#include "limine.h"
#include "kernel/mm/vmm.h"

static struct limine_memmap_request s_memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_framebuffer_request s_framebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct ts_kernelBootInfo s_kernelBootInfo;

static void initializeMemoryMap(struct ts_kernelBootInfo *p_bootInfo);
static void initializeFramebuffer(struct ts_kernelBootInfo *p_bootInfo);
static void panic(const char *p_message);

static uint8_t s_kernelStack[16384];

void _start(void) {
    kernelDebug("AnkeOS kernel - x86_64-limine bootstrap\n");

    // Initialize stack
    asm volatile("movq %0, %%rsp" :: "a"(&s_kernelStack[sizeof(s_kernelStack)]));

    initializeMemoryMap(&s_kernelBootInfo);
    initializeFramebuffer(&s_kernelBootInfo);

    kernelMain(&s_kernelBootInfo);

    panic("kernelMain() returned");

    while(1);
}

const struct ts_kernelBootInfo *kernelGetBootInfo(void) {
    return &s_kernelBootInfo;
}

static void initializeMemoryMap(struct ts_kernelBootInfo *p_bootInfo) {
    // If the memory map was not provided by Limine, panic.
    if(s_memmapRequest.response == NULL) {
        panic("Limine did not provide a memory map.");
    }

    // Compute the size of the memory map.
    size_t l_memoryMapSize = sizeof(struct limine_memmap_entry) * (s_memmapRequest.response->entry_count + 1lu);

    // Page align the size of the memory map
    l_memoryMapSize = (l_memoryMapSize + 0xffflu) & ~(0xffflu);

    // Find a free memory space to contain the memory map
    int l_freeEntryIndex = -1;

    for(uint64_t l_entryIndex = 0; l_entryIndex < s_memmapRequest.response->entry_count; l_entryIndex++) {
        if(s_memmapRequest.response->entries[l_entryIndex]->type == LIMINE_MEMMAP_USABLE) {
            if(s_memmapRequest.response->entries[l_entryIndex]->length >= l_memoryMapSize) {
                l_freeEntryIndex = (int)l_entryIndex;
                break;
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
            l_kernelMemoryMap[l_entryWriteIndex].m_base = s_memmapRequest.response->entries[l_entryReadIndex]->base;
            l_kernelMemoryMap[l_entryWriteIndex].m_size = l_memoryMapSize;
            l_kernelMemoryMap[l_entryWriteIndex].m_type = E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE;
            l_entryWriteIndex++;

            l_kernelMemoryMap[l_entryWriteIndex].m_base = s_memmapRequest.response->entries[l_entryReadIndex]->base + l_memoryMapSize;
            l_kernelMemoryMap[l_entryWriteIndex].m_size = s_memmapRequest.response->entries[l_entryReadIndex]->length - l_memoryMapSize;
        } else {
            l_kernelMemoryMap[l_entryWriteIndex].m_base = s_memmapRequest.response->entries[l_entryReadIndex]->base;
            l_kernelMemoryMap[l_entryWriteIndex].m_size = s_memmapRequest.response->entries[l_entryReadIndex]->length;
        }

        switch(s_memmapRequest.response->entries[l_entryReadIndex]->type) {
            case LIMINE_MEMMAP_USABLE:
                l_kernelMemoryMap[l_entryWriteIndex].m_type = E_KERNELMEMORYMAPENTRYTYPE_FREE;
                break;

            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                l_kernelMemoryMap[l_entryWriteIndex].m_type = E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE;
                break;

            default:
                l_kernelMemoryMap[l_entryWriteIndex].m_type = E_KERNELMEMORYMAPENTRYTYPE_RESERVED;
                break;
        }

        l_entryWriteIndex++;
    }

    p_bootInfo->memoryMap = l_kernelMemoryMap;
    p_bootInfo->memoryMapEntryCount = s_memmapRequest.response->entry_count + 1;
}

static void initializeFramebuffer(struct ts_kernelBootInfo *p_bootInfo) {
    if(
        (s_framebufferRequest.response == NULL)
        || (s_framebufferRequest.response->framebuffer_count == 0)
    ) {
        p_bootInfo->m_framebufferInfo.m_address = NULL;
        return;
    }

    struct limine_framebuffer *l_framebuffer =
        s_framebufferRequest.response->framebuffers[0];

    p_bootInfo->m_framebufferInfo.m_address = 
        vmmGetPhysicalAddress(l_framebuffer->address);
    p_bootInfo->m_framebufferInfo.m_width = l_framebuffer->width;
    p_bootInfo->m_framebufferInfo.m_height = l_framebuffer->height;
    p_bootInfo->m_framebufferInfo.m_pitch = l_framebuffer->pitch;
    p_bootInfo->m_framebufferInfo.m_bitsPerPixel = l_framebuffer->bpp;
    p_bootInfo->m_framebufferInfo.m_redMaskSize = l_framebuffer->red_mask_size;
    p_bootInfo->m_framebufferInfo.m_redMaskShift =
        l_framebuffer->red_mask_shift;
    p_bootInfo->m_framebufferInfo.m_greenMaskSize =
        l_framebuffer->green_mask_size;
    p_bootInfo->m_framebufferInfo.m_greenMaskShift = 
        l_framebuffer->green_mask_shift;
    p_bootInfo->m_framebufferInfo.m_blueMaskSize =
        l_framebuffer->blue_mask_size;
    p_bootInfo->m_framebufferInfo.m_blueMaskShift =
        l_framebuffer->blue_mask_shift;
}

static void panic(const char *p_message) {
    kernelDebug("Kernel panic: %s\n", p_message);

    while(1) {
        cli();
        hlt();
    }
}

#endif
