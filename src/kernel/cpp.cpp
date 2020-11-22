#include <stddef.h>
#include "libk/libk.hpp"
#include "sync/mutex.hpp"

#include "panic.hpp"

extern "C" void __cxa_pure_virtual() {
    kernel::panic("__cxa_pure_virtual() was called");
}

namespace __cxxabiv1 {
	/* guard variables */
 
	/* The ABI requires a 64-bit type.  */
	__extension__ typedef int __guard __attribute__((mode(__DI__)));
 
	extern "C" int __cxa_guard_acquire (__guard *);
	extern "C" void __cxa_guard_release (__guard *);
	extern "C" void __cxa_guard_abort (__guard *);
 
	extern "C" int __cxa_guard_acquire (__guard *g) 
	{
		return !kernel::mutex_tryAcquire((kernel::mutex_t *)g);
		//return !*(char *)(g);
	}
 
	extern "C" void __cxa_guard_release (__guard *g)
	{
		kernel::mutex_release((kernel::mutex_t *)g);
		//*(char *)g = 1;
	}
 
	extern "C" void __cxa_guard_abort (__guard *)
	{
 
	}
}

void *operator new(size_t size) {
    return std::malloc(size, true);
}
 
void *operator new[](size_t size) {
    return std::malloc(size, true);
}
 
void operator delete(void *p) {
    std::free(p);
}
 
void operator delete[](void *p) {
    std::free(p);
}
 
void operator delete(void *p, size_t s) {
	UNUSED_PARAMETER(s);
    std::free(p);
}
 
void operator delete[](void *p, size_t s) {
	UNUSED_PARAMETER(s);
    std::free(p);
}
