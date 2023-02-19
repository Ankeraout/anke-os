#include <stddef.h>

#include <kernel/klibc/list.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/klibc/string.h>
#include <kernel/mm/pmm.h>

static struct ts_list *listResize(
    struct ts_list *p_list,
    size_t p_newSize
);

struct ts_list *listInit(
    struct ts_list *p_list
) {
    p_list->a_data = NULL;
    p_list->a_length = 0;
    p_list->a_size = 0;

    return p_list;
}

struct ts_list *listAdd(struct ts_list *p_list, void *p_element) {
    if(p_list->a_length == p_list->a_size) {
        size_t l_newSize;

        if(p_list->a_size == 0) {
            l_newSize = 4096 / sizeof(void *);
        } else {
            l_newSize = p_list->a_size * 2;
        }

        if(listResize(p_list, l_newSize) == NULL) {
            return NULL;
        }
    }

    p_list->a_data[p_list->a_length++] = p_element;

    return p_list;
}

struct ts_list *listRemove(struct ts_list *p_list, size_t p_index) {
    if(p_list->a_length == 0) {
        return NULL;
    }

    for(size_t l_index = p_index; l_index < p_list->a_length - 1; l_index++) {
        p_list->a_data[l_index] = p_list->a_data[l_index + 1];
    }

    p_list->a_length--;

    return p_list;
}

size_t listGetSize(struct ts_list *p_list) {
    return p_list->a_size;
}

size_t listGetLength(struct ts_list *p_list) {
    return p_list->a_length;
}

void *listGet(struct ts_list *p_list, size_t p_index) {
    if(p_index >= p_list->a_length) {
        return NULL;
    }

    return p_list->a_data[p_index];
}

static struct ts_list *listResize(
    struct ts_list *p_list,
    size_t p_newSize
) {
    const size_t l_previousSize = p_list->a_size;
    const size_t l_previousSizeBytes = l_previousSize * sizeof(void *);
    const size_t l_newSizeBytes = p_newSize * sizeof(void *);

    /* TODO: change when the memory manager will be more advanced. */
    void **l_newData = pmmAlloc(l_newSizeBytes);

    if(l_newData == NULL) {
        return NULL;
    }

    memcpy(l_newData, p_list->a_data, p_list->a_length * sizeof(void *));

    pmmFree(p_list->a_data, l_previousSizeBytes);

    p_list->a_data = l_newData;

    return p_list;
}
