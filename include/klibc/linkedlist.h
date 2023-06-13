#ifndef __INCLUDE_KLIBC_LINKEDLIST_H__
#define __INCLUDE_KLIBC_LINKEDLIST_H__

struct ts_linkedListNode {
    void *data;
    struct ts_linkedListNode *next;
};

#endif
