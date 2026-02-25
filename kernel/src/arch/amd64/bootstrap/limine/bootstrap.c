#include <stddef.h>

#include "acpi/acpi.h"
#include "arch/amd64/asm.h"
#include "bootstrap.h"
#include "limine.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "printk.h"
#include "memoryRange.h"

#define C_MAX_MEMORY_MAP_ENTRIES 128

struct limine_memmap_request g_memoryMapRequest = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

struct limine_hhdm_request g_hhdmRequest = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 3,
    .response = NULL
};

struct limine_executable_address_request g_executableAddressRequest = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

static struct ts_bootstrap_information s_bootstrap_information;
static struct ts_bootstrap_memoryMapEntry s_memoryMap[C_MAX_MEMORY_MAP_ENTRIES];
static size_t s_memoryMapEntryCount;

static void halt(void);

void bootstrap_init(void) {
    if(g_memoryMapRequest.response == NULL) {
        pr_crit("bootstrap: No memory map provided by the bootloader.\n");
        halt();
    }

    if(g_hhdmRequest.response == NULL) {
        pr_crit("bootstrap: No HHDM pointer provided by the bootloader.\n");
        halt();
    }

    if(g_executableAddressRequest.response == NULL) {
        pr_crit("bootstrap: No kernel address provided by the bootloader.\n");
        halt();
    }

    s_memoryMapEntryCount = 0;

    for(
        unsigned int l_i = 0;
        l_i < g_memoryMapRequest.response->entry_count;
        l_i++
    ) {
        const uint64_t l_type = g_memoryMapRequest.response->entries[l_i]->type;

        if(
            (l_type == LIMINE_MEMMAP_USABLE)
            || (l_type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)
            || (l_type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES)
        ) {
            if(s_memoryMapEntryCount == C_MAX_MEMORY_MAP_ENTRIES) {
                pr_warning("bootstrap: Too many memory map entries.\n");
                break;
            }

            s_memoryMap[s_memoryMapEntryCount].m_range.m_ptr =
                (void *)g_memoryMapRequest.response->entries[l_i]->base;
            s_memoryMap[s_memoryMapEntryCount].m_range.m_size =
                (size_t)g_memoryMapRequest.response->entries[l_i]->length;

            enum te_bootstrap_memoryMapEntryType l_type2;
            
            if(l_type == LIMINE_MEMMAP_USABLE) {
                l_type2 = E_BOOTSTRAP_MEMORYMAPENTRYTYPE_FREE;
            } else {
                l_type2 = E_BOOTSTRAP_MEMORYMAPENTRYTYPE_RESERVED;
            }

            s_memoryMap[s_memoryMapEntryCount].m_type = l_type2;

            s_memoryMapEntryCount++;
        }
    }

    s_bootstrap_information.m_memoryMap.m_memoryMap = s_memoryMap;
    s_bootstrap_information.m_memoryMap.m_memoryMapLength =
        s_memoryMapEntryCount;
    s_bootstrap_information.m_hhdm = (void *)g_hhdmRequest.response->offset;
    s_bootstrap_information.m_kernelAddress.m_physicalAddress =
        (void *)g_executableAddressRequest.response->physical_base;
    s_bootstrap_information.m_kernelAddress.m_virtualAddress =
        (void *)g_executableAddressRequest.response->virtual_base;
}

const struct ts_bootstrap_information *bootstrap_getInformation(void) {
    return &s_bootstrap_information;
}

static void halt(void) {
    while(1) {
        asm_cli();
        asm_hlt();
    }
}
