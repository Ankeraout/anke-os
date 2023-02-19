#ifndef __INCLUDE_KERNEL_MISC_LIST_H__
#define __INCLUDE_KERNEL_MISC_LIST_H__

#include <stddef.h>
#include <kernel/misc/list.h>

struct ts_arrayList {
    size_t a_size;
    size_t a_length;
    void **a_data;
};

struct ts_linkedListNode {
    struct ts_linkedListNode *a_previous;
    struct ts_linkedListNode *a_next;
    void *a_data;
};

struct ts_linkedList {
    struct ts_linkedListNode *a_first;
    struct ts_linkedListNode *a_last;
    size_t a_length;
};

int arrayListInit(struct ts_arrayList *p_list);
int arrayListAdd(struct ts_arrayList *p_list, void *p_element);
int arrayListRemove(struct ts_arrayList *p_list, size_t p_index);
int arrayListRemoveItem(struct ts_arrayList *p_list, const void *p_element);
void arrayListDestroy(struct ts_arrayList *p_list);
void arrayListClear(struct ts_arrayList *p_list);
void *arrayListGet(struct ts_arrayList *p_list, size_t p_index);
size_t arrayListGetLength(struct ts_arrayList *p_list);
int linkedListInit(struct ts_linkedList *p_list);
int linkedListAdd(struct ts_linkedList *p_list, void *p_element);
int linkedListRemove(struct ts_linkedList *p_list, size_t p_index);
int linkedListRemoveNode(
    struct ts_linkedList *p_list,
    struct ts_linkedListNode *p_node
);
int linkedListRemoveItem(struct ts_linkedList *p_list, const void *p_element);
size_t linkedListGetLength(struct ts_linkedList *p_list);
void linkedListDestroy(struct ts_linkedList *p_list);
void linkedListClear(struct ts_linkedList *p_list);
void *linkedListGet(struct ts_linkedList *p_list, size_t p_index);

#endif
