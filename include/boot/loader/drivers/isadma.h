#ifndef __INCLUDE_BOOT_LOADER_DRIVERS_ISADMA_H__
#define __INCLUDE_BOOT_LOADER_DRIVERS_ISADMA_H__

#include <stddef.h>

void isadma_init(int p_channel, size_t p_size);
void isadma_init_read(int p_channel);
void isadma_read(void *p_buffer, size_t p_size);
void isadma_init_write(int p_channel, void *p_buffer, size_t p_size);

#endif
