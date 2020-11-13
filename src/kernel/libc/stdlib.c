#include <stdbool.h>
#include <stddef.h>

#include "libc/stdlib.h"
#include "libk/libk.h"
#include "syscall.h"

void *malloc(size_t size) {
    return (void *)syscall_call(SYSCALL_MALLOC, size);
}
