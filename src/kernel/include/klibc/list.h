#ifndef __INCLUDE_KLIBC_LIST_H__
#define __INCLUDE_KLIBC_LIST_H__

#include <stddef.h>

struct ts_list {
    size_t a_size;
    size_t a_length;
    void **a_data;
};

struct ts_list *listInit(struct ts_list *p_list);
struct ts_list *listAdd(struct ts_list *p_list, void *p_element);
struct ts_list *listRemove(struct ts_list *p_list, size_t p_index);
size_t listGetSize(struct ts_list *p_list);
size_t listGetLength(struct ts_list *p_list);
void *listGet(struct ts_list *p_list, size_t p_index);

#endif
