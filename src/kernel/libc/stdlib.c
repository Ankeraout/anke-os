#include <stdbool.h>
#include <stddef.h>

#include "libc/stdlib.h"
#include "libk/libk.h"
#include "syscall.h"

void *malloc(size_t size) {
    void *ret = (void *)syscall_call(SYSCALL_MALLOC, size);
}
