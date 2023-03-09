#include <stdbool.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/boot/boot.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <limine.h>

#ifdef __BOOT_LIMINE

static volatile struct limine_memmap_request s_memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_framebuffer_request s_framebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void bootDebugWrite(void *p_parameter, char p_value);

void _start(void) {
    struct ts_bootMemoryMapEntry l_memoryMap[s_memmapRequest.response->entry_count];

    for(
        size_t l_entryIndex = 0;
        l_entryIndex < s_memmapRequest.response->entry_count;
        l_entryIndex++
    ) {
        l_memoryMap[l_entryIndex].a_base = s_memmapRequest.response->entries[l_entryIndex]->base;
        l_memoryMap[l_entryIndex].a_size = s_memmapRequest.response->entries[l_entryIndex]->length;

        uint64_t l_entryType = s_memmapRequest.response->entries[l_entryIndex]->type;

        if(l_entryType == LIMINE_MEMMAP_USABLE) {
            l_memoryMap[l_entryIndex].a_type = E_MMAP_TYPE_FREE;
        } else if(l_entryType == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            l_memoryMap[l_entryIndex].a_type = E_MMAP_TYPE_RECLAIMABLE;
        } else if(l_entryType == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            l_memoryMap[l_entryIndex].a_type = E_MMAP_TYPE_KERNEL;
        } else {
            l_memoryMap[l_entryIndex].a_type = E_MMAP_TYPE_RESERVED;
        }
    }

    struct ts_boot l_boot = {
        .a_memoryMap = l_memoryMap,
        .a_memoryMapLength = s_memmapRequest.response->entry_count,
        .a_framebuffer = {
            .a_buffer = s_framebufferRequest.response->framebuffers[0]->address,
            .a_width = s_framebufferRequest.response->framebuffers[0]->width,
            .a_height = s_framebufferRequest.response->framebuffers[0]->height,
            .a_pitch = s_framebufferRequest.response->framebuffers[0]->pitch
        }
    };

    debugInit(bootDebugWrite, NULL);

    main(&l_boot);
}

static void bootDebugWrite(void *p_parameter, char p_value) {
    M_UNUSED_PARAMETER(p_parameter);

    outb(0xe9, p_value);
}

#endif
