#ifndef __KERNEL_SYNC_MUTEX_HPP__
#define __KERNEL_SYNC_MUTEX_HPP__

namespace kernel {
    typedef int mutex_t;

    extern "C" {
        extern void mutex_acquire(mutex_t *mutex);
        extern void mutex_release(mutex_t *mutex);

        /**
         * Summary:
         *  Tries to acquire the given mutex.
         * 
         * Arguments:
         *  - mutex: The mutex to acquire
         * 
         * Return value:
         *  0 if the mutex was acquired, any other value otherwise.
         */ 
        extern int mutex_tryAcquire(mutex_t *mutex);
    };
}

#endif
