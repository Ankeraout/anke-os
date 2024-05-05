#ifndef __INCLUDE_KERNEL_KMALLOC_H__
#define __INCLUDE_KERNEL_KMALLOC_H__

#include <stddef.h>

void *kmalloc(size_t p_size, int p_flags);
void kfree(void *p_ptr);

#endif
