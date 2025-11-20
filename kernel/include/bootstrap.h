#ifndef __INCLUDE_BOOTSTRAP_H__
#define __INCLUDE_BOOTSTRAP_H__

#include <stddef.h>
#include "memoryRange.h"

enum te_bootstrap_memoryMapEntryType {
    E_BOOTSTRAP_MEMORYMAPENTRYTYPE_FREE,
    E_BOOTSTRAP_MEMORYMAPENTRYTYPE_RESERVED
};

struct ts_bootstrap_memoryMapEntry {
    enum te_bootstrap_memoryMapEntryType m_type;
    struct ts_memoryRange m_range;
};

struct ts_bootstrap_memoryMap {
    struct ts_bootstrap_memoryMapEntry *m_memoryMap;
    size_t m_memoryMapLength;
};

struct ts_bootstrap_kernelAddress {
    void *m_physicalAddress;
    void *m_virtualAddress;
};

struct ts_bootstrap_information {
    struct ts_bootstrap_memoryMap m_memoryMap;
    void *m_hhdm;
    struct ts_bootstrap_kernelAddress m_kernelAddress;
};

void bootstrap_init(void);
const struct ts_bootstrap_information *bootstrap_getInformation(void);

#endif
