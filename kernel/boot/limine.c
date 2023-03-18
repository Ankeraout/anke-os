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

struct ts_boot s_boot;

static void bootDebugWrite(void *p_parameter, const char *p_str);

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

    s_boot.a_memoryMap = l_memoryMap;
    s_boot.a_memoryMapLength = s_memmapRequest.response->entry_count;
    s_boot.a_framebuffer.a_buffer
        = s_framebufferRequest.response->framebuffers[0]->address;
    s_boot.a_framebuffer.a_width
        = s_framebufferRequest.response->framebuffers[0]->width;
    s_boot.a_framebuffer.a_height
        = s_framebufferRequest.response->framebuffers[0]->height;
    s_boot.a_framebuffer.a_pitch
        = s_framebufferRequest.response->framebuffers[0]->pitch;

    debugInit(bootDebugWrite, NULL);

    main(&s_boot);
}

const struct ts_boot *bootGetInfo(void) {
    return &s_boot;
}

static void bootDebugWrite(void *p_parameter, const char *p_str) {
    M_UNUSED_PARAMETER(p_parameter);

    size_t l_index = 0;

    while(p_str[l_index] != '\0') {
        outb(0xe9, p_str[l_index]);
        l_index++;
    }

}

#endif
