#ifndef __KERNEL_SYSCALL_H__
#define __KERNEL_SYSCALL_H__

#include <stddef.h>

enum {
    SYSCALL_MALLOC
};

void syscall(size_t function, size_t argument);

#endif
