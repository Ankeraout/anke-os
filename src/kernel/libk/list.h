#ifndef __KERNEL_LIBK_LIST_H__
#define __KERNEL_LIBK_LIST_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t length;
    size_t internalLength;
    void **elements;
} list_t;

list_t *list_create();
void list_add(list_t *list, void *element);
void list_remove(list_t *list, void *element);
void list_removeAt(list_t *list, size_t index);
void list_insert(list_t *list, size_t index, void *element);
void list_destroy(list_t *list);

#endif
