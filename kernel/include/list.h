#ifndef __INCLUDE_LIST_H__
#define __INCLUDE_LIST_H__

struct ts_list_node {
    struct ts_list_node *m_next;
    void *m_data;
};

int list_insertBeginning(struct ts_list_node **p_list, void *p_data);
void list_remove(struct ts_list_node **p_list, void *p_data);

#endif
