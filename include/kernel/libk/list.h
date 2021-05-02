#ifndef __KERNEL_LIBK_LIST_H__
#define __KERNEL_LIBK_LIST_H__

#include "kernel/libk/stdlib.h"

#define list_add(__list, __element) {\
    list_t *__newCell = malloc(sizeof(list_t)); \
    __newCell->next = __list; \
    __newCell->element = (void *)__element; \
    __list = __newCell; \
}

typedef struct list_s {
    void *element;
    struct list_s *next;
} list_t;

#endif
