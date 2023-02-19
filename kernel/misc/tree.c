#include <stddef.h>

#include <kernel/klibc/stdlib.h>
#include <kernel/misc/list.h>
#include <kernel/misc/tree.h>

int treeInit(struct ts_treeNode *p_tree, void *p_data) {
    p_tree->a_parent = NULL;
    p_tree->a_data = p_data;
    return linkedListInit(&p_tree->a_children);
}

int treeAddChild(struct ts_treeNode *p_tree, void *p_data) {
    struct ts_treeNode *l_newNode = kmalloc(sizeof(struct ts_treeNode));

    if(l_newNode == NULL) {
        return 1;
    }

    if(treeInit(l_newNode, p_data) != 0) {
        kfree(l_newNode);
        return 1;
    }

    if(linkedListAdd(&p_tree->a_children, l_newNode) != 0) {
        kfree(l_newNode);
        return 1;
    }

    l_newNode->a_parent = p_tree;

    return 0;
}

int treeDestroy(struct ts_treeNode *p_tree) {
    struct ts_linkedListNode *l_currentChild = p_tree->a_children.a_first;

    while(l_currentChild != NULL) {
        treeDestroy((struct ts_treeNode *)l_currentChild->a_data);
        l_currentChild = l_currentChild->a_next;
    }

    return linkedListRemoveItem(&p_tree->a_parent->a_children, p_tree);
}

size_t treeGetLength(struct ts_treeNode *p_tree) {
    if(p_tree == NULL) {
        return 0;
    }

    struct ts_linkedListNode *l_currentChild = p_tree->a_children.a_first;
    size_t l_length = 1;

    while(l_currentChild != NULL) {
        l_length += treeGetLength((struct ts_treeNode *)l_currentChild->a_data);
        l_currentChild = l_currentChild->a_next;
    }

    return l_length;
}
