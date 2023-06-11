#include <stdbool.h>

#include "kernel/arch/x86_64/inline.h"
#include "kernel/arch.h"
#include "kernel/boot.h"
#include "klibc/debug.h"

static int kernelPreinit(const struct ts_kernelBootInfo *p_bootInfo);
static int kernelInit(void);

void kernelMain(const struct ts_kernelBootInfo *p_bootInfo) {
    int l_returnValue = archPreinit(p_bootInfo);

    if(l_returnValue != 0) {
        kernelDebug("archPreinit() failed with code %d.\n", l_returnValue);
        return;
    }

    l_returnValue = kernelPreinit(p_bootInfo);

    if(l_returnValue != 0) {
        kernelDebug("kernelPreinit() failed with code %d.\n", l_returnValue);
        return;
    }

    l_returnValue = archInit();

    if(l_returnValue != 0) {
        kernelDebug("archInit() failed with code %d.\n", l_returnValue);
        return;
    }

    l_returnValue = kernelInit();

    if(l_returnValue != 0) {
        kernelDebug("kernelInit() failed with code %d.\n", l_returnValue);
        return;
    }
}

static int kernelPreinit(const struct ts_kernelBootInfo *p_bootInfo) {
    kernelDebug("\nMemory map:\n\n");
    kernelDebug("Base               | Length             | Type\n");
    kernelDebug("-------------------+--------------------+-----\n");

    const char *l_type = NULL;

    for(size_t l_index = 0; l_index < p_bootInfo->memoryMapEntryCount; l_index++) {
        const struct ts_kernelMemoryMapEntry *l_memoryMapEntry = &p_bootInfo->memoryMap[l_index];

        switch(l_memoryMapEntry->type) {
            case E_KERNELMEMORYMAPENTRYTYPE_FREE: l_type = "Free"; break;
            case E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE: l_type = "Reclaimable"; break;
            case E_KERNELMEMORYMAPENTRYTYPE_RESERVED: l_type = "Reserved"; break;
            default: l_type = "Unknown"; break;
        }

        kernelDebug("0x%016lx | 0x%016lx | %s\n", l_memoryMapEntry->base, l_memoryMapEntry->length, l_type);
    }

    kernelDebug("\n");

    return 0;
}

static int kernelInit(void) {
    kernelDebug("kernelInit() called.\n");

    return 0;
}
