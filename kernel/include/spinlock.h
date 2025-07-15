#ifndef __INCLUDE_SPINLOCK_H__
#define __INCLUDE_SPINLOCK_H__

#include <stddef.h>

typedef size_t t_spinlock;

void spinlock_acquire(t_spinlock *p_spinlock);
void spinlock_release(t_spinlock *p_spinlock);

#endif
