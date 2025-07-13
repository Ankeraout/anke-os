#ifndef __INCLUDE_TREE_H__
#define __INCLUDE_TREE_H__

struct ts_treeNode {
    struct ts_treeNode *m_parent;
    struct ts_treeNode *m_next;
    struct ts_treeNode *m_children;
    void *m_data;
};

#endif
