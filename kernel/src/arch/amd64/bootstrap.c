#include <stddef.h>

#include "acpi/acpi.h"
#include "arch/amd64/asm.h"
#include "limine.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "printk.h"

#define C_MAX_MEMORY_MAP_ENTRIES 128

struct limine_memmap_request g_memoryMapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

struct limine_rsdp_request g_rsdpRequest = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
    .response = NULL
};

struct limine_smbios_request g_smbiosRequest = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0,
    .response = NULL
};

struct limine_framebuffer_request g_framebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct ts_mm_memoryMapEntry s_memoryMap[C_MAX_MEMORY_MAP_ENTRIES];

void bootstrap(void) {
    if(g_memoryMapRequest.response == NULL) {
        pr_crit("bootstrap: No memory map provided by the bootloader.\n");
        
        while(1) {
            asm_cli();
            asm_hlt();
        }
    }

    int l_memoryMapEntryCount = 0;

    for(
        unsigned int l_i = 0;
        l_i < g_memoryMapRequest.response->entry_count;
        l_i++
    ) {
        if(
            g_memoryMapRequest.response->entries[l_i]->type
            == LIMINE_MEMMAP_USABLE
        ) {
            if(l_memoryMapEntryCount == C_MAX_MEMORY_MAP_ENTRIES) {
                pr_warning("bootstrap: Too many memory map entries.\n");
                break;
            }

            s_memoryMap[l_memoryMapEntryCount].m_base =
                (void *)g_memoryMapRequest.response->entries[l_i]->base;
            s_memoryMap[l_memoryMapEntryCount].m_size =
                (size_t)g_memoryMapRequest.response->entries[l_i]->length;

            l_memoryMapEntryCount++;
        }
    }

    if(pmm_init(s_memoryMap, l_memoryMapEntryCount) != 0) {
        pr_crit("bootstrap: pmmInit() failed.\n");
        
        while(1) {
            asm_cli();
            asm_hlt();
        }
    }

    if(g_rsdpRequest.response != NULL) {
        pr_info("bootstrap: RSDP at %p\n", g_rsdpRequest.response->address);
        acpi_setRsdpLocation(g_rsdpRequest.response->address);
    } else {
        pr_info("bootstrap: No RSDP provided by the bootloader.\n");
    }
}
