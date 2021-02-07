#ifndef __KERNEL_TIME_H__
#define __KERNEL_TIME_H__

#include <stdint.h>

extern const volatile uint64_t kernel_timer;

void sleep(unsigned int milliseconds);

#endif
