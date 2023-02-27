#ifndef __INCLUDE_KERNEL_MISC_SPINLOCK_H__
#define __INCLUDE_KERNEL_MISC_SPINLOCK_H__

#include <stdint.h>

typedef uint32_t t_spinlock;

void spinlockAcquire(t_spinlock *p_spinlock);
void spinlockRelease(t_spinlock *p_spinlock);

#endif
