#ifndef __INCLUDE_KERNEL_UTIL_LIST_H__
#define __INCLUDE_KERNEL_UTIL_LIST_H__

#include <stdbool.h>

#include "klibc/errno.h"
#include "klibc/stdlib.h"

struct ts_list {
    void *m_data;
    struct ts_list *m_next;
    struct ts_list *m_previous;
};

static inline int listAdd(struct ts_list **p_list, void *p_element) {
    struct ts_list *l_newElement = kmalloc(sizeof(struct ts_list));

    if(l_newElement == NULL) {
        return -ENOMEM;
    }
    
    l_newElement->m_data = p_element;
    l_newElement->m_next = NULL;

    if(*p_list == NULL) {
        *p_list = l_newElement;
        return 0;
    }
    
    struct ts_list *l_currentElement = *p_list;

    while(l_currentElement->m_next != NULL) {
        l_currentElement = l_currentElement->m_next;
    }

    l_currentElement->m_next = l_newElement;
    l_newElement->m_previous = l_currentElement;

    return 0;
}

static inline int listRemove(struct ts_list **p_list, void *p_element) {
    // If the list is empty, there is nothing to do.
    if(*p_list == NULL) {
        return -1;
    }
    
    // If the element to remove is the first element in the list
    if((*p_list)->m_data == p_element) {
        struct ts_list *l_currentElement = *p_list;
        *p_list = l_currentElement->m_next;
        (*p_list)->m_previous = NULL;
        kfree(l_currentElement);
        return 0;
    }

    struct ts_list *l_previousElement = *p_list;
    struct ts_list *l_currentElement = l_previousElement->m_next;

    while(l_currentElement != NULL) {
        if(l_currentElement->m_data == p_element) {
            l_previousElement->m_next = l_currentElement->m_next;

            if(l_currentElement->m_next != NULL) {
                l_currentElement->m_next->m_previous = l_previousElement;
            }

            kfree(l_currentElement);
            return 0;
        }

        l_previousElement = l_currentElement;
        l_currentElement = l_currentElement->m_next;
    }

    return -1;
}

static inline bool listContains(struct ts_list *p_list, void *p_element) {
    while(p_list != NULL) {
        if(p_list->m_data == p_element) {
            return true;
        }

        p_list = p_list->m_next;
    }

    return false;
}

#endif
