#ifndef __INCLUDE_KERNEL_LINKEDLIST_H__
#define __INCLUDE_KERNEL_LINKEDLIST_H__

#include <stdbool.h>

struct ts_linkedList {
    void *m_data;
    struct ts_linkedList *m_previous;
    struct ts_linkedList *m_next;
};

int linkedListAdd(struct ts_linkedList **p_list, void *p_element);
int linkedListRemove(struct ts_linkedList **p_list, void *p_element);
bool linkedListContains(struct ts_linkedList *p_list, void *p_element);

#endif
