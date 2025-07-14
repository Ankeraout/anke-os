#include "errno.h"
#include "list.h"
#include "stdlib.h"

int list_insertBeginning(struct ts_list_node **p_list, void *p_data) {
    struct ts_list_node *l_node = malloc(sizeof(struct ts_list_node));

    if(l_node == NULL) {
        return -ENOMEM;
    }

    l_node->m_next = *p_list;
    l_node->m_data = p_data;

    *p_list = l_node;

    return 0;
}

void list_remove(struct ts_list_node **p_list, void *p_data) {
    struct ts_list_node *l_previousNode = NULL;
    struct ts_list_node *l_node = *p_list;

    while(l_node != NULL) {
        if(l_node->m_data == p_data) {
            if(l_previousNode == NULL) {
                *p_list = (*p_list)->m_next;
            } else {
                l_previousNode->m_next = l_node->m_next;
            }
        } else {
            l_previousNode = l_node;
        }

        l_node = l_node->m_next;
    }
}
