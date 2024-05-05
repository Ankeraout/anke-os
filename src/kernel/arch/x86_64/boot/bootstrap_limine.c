#include <stddef.h>

#include "kernel/arch/x86_64/stack.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/panic.h"
#include "kernel/printk.h"
#include "limine.h"

#define C_MMAP_MAX_ENTRIES 256

extern void main(void);

static struct limine_memmap_request s_memoryMapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_rsdp_request s_rsdpRequest = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_smbios_request s_smbiosRequest = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_framebuffer_request s_framebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct ts_mmMemoryMapEntry s_memoryMap[C_MMAP_MAX_ENTRIES];
static unsigned int s_memoryMapLength = 0U;
static uint8_t s_kernelStack[C_KERNEL_STACK_SIZE];

static int prepareMemoryMap(void);

void _start(void) {
    // Initialize stack
    asm("movq %0, %%rsp" :: "a"(s_kernelStack));

    if(prepareMemoryMap() != 0) {
        panic("bootstrap: Memory map initialization failed.\n");
    }

    if(pmmInit(s_memoryMap, (int)s_memoryMapLength) != 0) {
        panic("bootstrap: PMM initialization failed.\n");
    }

    main();
}

static int prepareMemoryMap(void) {
    if(s_memoryMapRequest.response == NULL) {
        pr_crit("bootstrap: No memory map.\n");
        return 1;
    }
    
    for(
        unsigned int l_i = 0;
        l_i < s_memoryMapRequest.response->entry_count;
        l_i++
    ) {
        if(
            s_memoryMapRequest.response->entries[l_i]->type
            == LIMINE_MEMMAP_USABLE
        ) {
            if(s_memoryMapLength == C_MMAP_MAX_ENTRIES) {
                pr_warning("bootstrap: Too many usable memory map entries.\n");
                return 0;
            }

            s_memoryMap[s_memoryMapLength].m_base =
                (void *)s_memoryMapRequest.response->entries[l_i]->base;
            s_memoryMap[s_memoryMapLength].m_size =
                (size_t)s_memoryMapRequest.response->entries[l_i]->length;
            s_memoryMapLength++;
        }
    }

    return 0;
}
