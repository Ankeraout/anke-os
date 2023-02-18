#ifndef __INCLUDE_KLIBC_STDLIB_H__
#define __INCLUDE_KLIBC_STDLIB_H__

#include <stddef.h>

void *kmalloc(size_t p_size);
void kfree(void *p_ptr);

#endif
