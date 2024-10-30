#ifndef __INCLUDE_BOOT_LOADER_MM_H__
#define __INCLUDE_BOOT_LOADER_MM_H__

#include <stddef.h>

#include "boot/loader/boot.h"

int mm_init(const struct ts_bootInfoStructure *p_bootInfoStructure);
void *mm_alloc(size_t p_size);
void mm_free(void *p_ptr, size_t p_size);

#endif
