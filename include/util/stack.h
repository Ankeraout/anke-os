#ifndef __INCLUDE_UTIL_STACK_H__
#define __INCLUDE_UTIL_STACK_H__

#include "klibc/stdlib.h"
#include "util/list.h"

static inline int stackPush(struct ts_list **p_list, void *p_element) {
    struct ts_list *l_newElement = kmalloc(sizeof(struct ts_list));

    if(l_newElement == NULL) {
        return -ENOMEM;
    }
    
    l_newElement->m_data = p_element;
    l_newElement->m_next = *p_list;
    l_newElement->m_previous = NULL;

    *p_list = l_newElement;

    return 0;
}

static inline int stackPop(struct ts_list **p_list, void **p_element) {
    if(*p_list == NULL) {
        return -1;
    }

    struct ts_list *l_currentElement = *p_list;

    *p_element = l_currentElement->m_data;
    *p_list = (*p_list)->m_next;

    if(*p_list != NULL) {
        (*p_list)->m_previous = NULL;
    }

    kfree(l_currentElement);

    return 0;
}

#endif
