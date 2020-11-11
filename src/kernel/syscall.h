#ifndef __SYSCALL_H__
#define __SYSCALL_H__

enum {
    SYSCALL_MALLOC
};

extern size_t syscall_call(size_t function, size_t argument);
extern void syscall_wrapper();

#endif
