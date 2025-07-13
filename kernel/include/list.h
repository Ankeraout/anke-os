#ifndef __INCLUDE_LIST_H__
#define __INCLUDE_LIST_H__

struct ts_listNode {
    struct ts_listNode *m_next;
    void *m_data;
};

int list_insertBeginning(struct ts_listNode **p_list, void *p_data);
void list_remove(struct ts_listNode **p_list, void *p_data);

#endif
