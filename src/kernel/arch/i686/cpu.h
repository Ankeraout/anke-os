#ifndef __KERNEL_ARCH_I686_CPU_H__
#define __KERNEL_ARCH_I686_CPU_H__

static inline void invlpg(const void* ptr) {
    asm volatile("invlpg (%0)" : : "b"(ptr) : "memory");
}

#endif
