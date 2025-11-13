#ifndef __INCLUDE_BOOTSTRAP_H__
#define __INCLUDE_BOOTSTRAP_H__

#include <stddef.h>
#include "memoryRange.h"

void bootstrap(void);
void bootstrap_getMemoryMap(
    struct ts_memoryRange **p_memoryMapEntries,
    size_t *p_memoryMapEntryCount
);

#endif
