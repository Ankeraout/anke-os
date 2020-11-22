#ifndef __KERNEL_SYSCALL_HPP__
#define __KERNEL_SYSCALL_HPP__

namespace kernel {
    enum {
        SYSCALL_MALLOC
    };

    extern "C" size_t syscall_call(size_t function, size_t argument);
}

#endif
