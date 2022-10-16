#include <stdbool.h>

#include "boot/boot.h"
#include "limine.h"

#ifdef __BOOT_LIMINE

static volatile struct limine_terminal_request s_terminalRequest = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request s_memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static void printHex(uint64_t p_value) {
    uint64_t l_value = p_value;
    
    char l_buffer[16];

    for(int l_i = 0; l_i < 16; l_i++) {
        uint64_t l_digit = l_value >> 60;

        if(l_digit <= 9) {
            l_buffer[l_i] = l_digit + '0';
        } else {
            l_buffer[l_i] = l_digit - 10 + 'a';
        }

        l_value <<= 4;
    }

    s_terminalRequest.response->write(s_terminalRequest.response->terminals[0], l_buffer, 16);
}

static void printMemoryMap(void) {
    s_terminalRequest.response->write(s_terminalRequest.response->terminals[0], "Memory map:", 11);

    for(uint64_t l_i = 0; l_i < s_memmapRequest.response->entry_count; l_i++) {
        s_terminalRequest.response->write(s_terminalRequest.response->terminals[0], "\nBase: 0x", 9);
        printHex(s_memmapRequest.response->entries[l_i]->base);
        s_terminalRequest.response->write(s_terminalRequest.response->terminals[0], ", size: 0x", 10);
        printHex(s_memmapRequest.response->entries[l_i]->length);
        s_terminalRequest.response->write(s_terminalRequest.response->terminals[0], ", type: 0x", 10);
        printHex(s_memmapRequest.response->entries[l_i]->type);
    }

    s_terminalRequest.response->write(s_terminalRequest.response->terminals[0], "\n", 1);
}

void _start(void) {
    printMemoryMap();
    
    // TODO: allocate this in the heap?
    uint8_t l_memoryMapBuffer[
        sizeof(struct limine_memmap_entry)
        * s_memmapRequest.response->entry_count
    ];

    struct ts_bootMemoryMapEntry *l_memoryMap =
        (struct ts_bootMemoryMapEntry *)l_memoryMapBuffer;

    for(
        size_t l_entryIndex = 0;
        l_entryIndex < s_memmapRequest.response->entry_count;
        l_entryIndex++
    ) {
        l_memoryMap[l_entryIndex].base = s_memmapRequest.response->entries[l_entryIndex]->base;
        l_memoryMap[l_entryIndex].size = s_memmapRequest.response->entries[l_entryIndex]->length;
        
        uint64_t l_entryType = s_memmapRequest.response->entries[l_entryIndex]->type;

        if(l_entryType == LIMINE_MEMMAP_USABLE) {
            l_memoryMap[l_entryIndex].type = E_MMAP_TYPE_FREE;
        } else if(l_entryType == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            l_memoryMap[l_entryIndex].type = E_MMAP_TYPE_RECLAIMABLE;
        } else if(l_entryType == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            l_memoryMap[l_entryIndex].type = E_MMAP_TYPE_KERNEL;
        } else {
            l_memoryMap[l_entryIndex].type = E_MMAP_TYPE_RESERVED;
        }
    }

    struct ts_boot l_boot = {
        .memoryMap = l_memoryMap,
        .memoryMapLength = s_memmapRequest.response->entry_count
    };

    main(&l_boot);
}

#endif
