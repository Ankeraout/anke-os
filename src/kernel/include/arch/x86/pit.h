#ifndef __INCLUDE_ARCH_X86_PIT_H__
#define __INCLUDE_ARCH_X86_PIT_H__

#include <stdbool.h>
#include <stdint.h>

void pitInit(void);
void pitSleep(uint64_t p_milliseconds);
uint64_t pitGetTimeMilliseconds(void);

#endif
