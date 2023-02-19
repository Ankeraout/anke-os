#ifndef __INCLUDE_KERNEL_MISC_TREE_H__
#define __INCLUDE_KERNEL_MISC_TREE_H__

#include <stddef.h>
#include <kernel/misc/list.h>

struct ts_treeNode {
    struct ts_treeNode *a_parent;
    struct ts_linkedList a_children;
    void *a_data;
};

int treeInit(struct ts_treeNode *p_tree, void *p_data);
int treeAddChild(struct ts_treeNode *p_tree, void *p_data);
int treeDestroy(struct ts_treeNode *p_tree);
size_t treeGetLength(struct ts_treeNode *p_tree);

#endif
