#include <stddef.h>

#include <kernel/klibc/stdlib.h>
#include <kernel/misc/list.h>

#define C_LIST_INITIAL_SIZE 1
#define C_LIST_ELEMENT_SIZE (sizeof(void *))
#define C_LIST_GROWTH_FACTOR 2

static int arrayListExtend(struct ts_arrayList *p_list);

int arrayListInit(struct ts_arrayList *p_list) {
    p_list->a_size = 0;
    p_list->a_length = 0;
    p_list->a_data = NULL;

    return 0;
}

int arrayListAdd(struct ts_arrayList *p_list, void *p_element) {
    if(p_list->a_length == p_list->a_size) {
        if(arrayListExtend(p_list) != 0) {
            return 1;
        }
    }

    p_list->a_data[p_list->a_length++] = p_element;

    return 0;
}

int arrayListRemove(struct ts_arrayList *p_list, size_t p_index) {
    if(p_index >= p_list->a_length) {
        return 1;
    }

    for(size_t l_index = p_index; l_index < p_list->a_length - 1; l_index++) {
        p_list->a_data[l_index] = p_list->a_data[l_index + 1];
    }

    p_list->a_length--;

    return 0;
}

int arrayListRemoveItem(struct ts_arrayList *p_list, const void *p_element) {
    for(size_t l_index = 0; l_index < p_list->a_length; l_index++) {
        if(p_list->a_data[l_index] == p_element) {
            return arrayListRemove(p_list, l_index);
        }
    }

    return 1;
}

void arrayListDestroy(struct ts_arrayList *p_list) {
    p_list->a_length = 0;
    p_list->a_size = 0;

    kfree(p_list->a_data);
}

void arrayListClear(struct ts_arrayList *p_list) {
    p_list->a_length = 0;
}

void *arrayListGet(struct ts_arrayList *p_list, size_t p_index) {
    if(p_index >= p_list->a_length) {
        return NULL;
    }

    return p_list->a_data[p_index];
}

size_t arrayListGetLength(struct ts_arrayList *p_list) {
    return p_list->a_length;
}

int linkedListInit(struct ts_linkedList *p_list) {
    p_list->a_first = NULL;
    p_list->a_last = NULL;
    p_list->a_length = 0;

    return 0;
}

int linkedListAdd(struct ts_linkedList *p_list, void *p_element) {
    struct ts_linkedListNode *l_newNode =
        kmalloc(sizeof(struct ts_linkedListNode));

    if(l_newNode == NULL) {
        return 1;
    }

    if(p_list->a_length == 0) {
        p_list->a_first = l_newNode;
        p_list->a_last = l_newNode;
        l_newNode->a_previous = NULL;
    } else {
        p_list->a_last->a_next = l_newNode;
        l_newNode->a_previous = p_list->a_last;
        p_list->a_last = l_newNode;
    }

    p_list->a_length++;

    l_newNode->a_data = p_element;
    l_newNode->a_next = NULL;

    return 0;
}

int linkedListRemove(struct ts_linkedList *p_list, size_t p_index) {
    if(p_index >= p_list->a_length) {
        return 1;
    }

    struct ts_linkedListNode *p_currentNode = p_list->a_first;

    for(size_t l_index = 1; l_index < p_index; l_index++) {
        p_currentNode = p_currentNode->a_next;
    }

    return linkedListRemoveNode(p_list, p_currentNode);
}

int linkedListRemoveNode(
    struct ts_linkedList *p_list,
    struct ts_linkedListNode *p_node
) {
    struct ts_linkedListNode *p_previousNode = p_node->a_previous;
    struct ts_linkedListNode *p_nextNode = p_node->a_next;

    if(p_previousNode != NULL) {
        p_previousNode->a_next = p_nextNode;
        p_list->a_first = p_nextNode;
    }

    if(p_nextNode != NULL) {
        p_nextNode->a_previous = p_previousNode;
        p_list->a_last = p_previousNode;
    }

    kfree(p_node);

    p_list->a_length--;

    return 0;
}

int linkedListRemoveItem(struct ts_linkedList *p_list, const void *p_element) {
    struct ts_linkedListNode *p_currentNode = p_list->a_first;

    while(p_currentNode != NULL) {
        if(p_currentNode->a_data == p_element) {
            linkedListRemoveNode(p_list, p_currentNode);
            return 0;
        }
    }

    return 1;
}

size_t linkedListGetLength(struct ts_linkedList *p_list) {
    return p_list->a_length;
}

void linkedListDestroy(struct ts_linkedList *p_list) {
    linkedListClear(p_list);
}

void linkedListClear(struct ts_linkedList *p_list) {
    struct ts_linkedListNode *p_currentNode = p_list->a_first;

    while(p_currentNode != NULL) {
        struct ts_linkedListNode *p_nextNode = p_currentNode->a_next;

        kfree(p_currentNode);

        p_currentNode = p_nextNode;
    }

    p_list->a_first = NULL;
    p_list->a_last = NULL;
    p_list->a_length = 0;
}

void *linkedListGet(struct ts_linkedList *p_list, size_t p_index) {
    if(p_index >= p_list->a_length) {
        return NULL;
    }

    struct ts_linkedListNode *p_currentNode = p_list->a_first;

    for(size_t l_index = 1; l_index < p_index; l_index++) {
        p_currentNode = p_currentNode->a_next;
    }

    return p_currentNode->a_data;
}

static int arrayListExtend(struct ts_arrayList *p_list) {
    if(p_list->a_size == 0) {
        p_list->a_data = kmalloc(C_LIST_INITIAL_SIZE * C_LIST_ELEMENT_SIZE);

        if(p_list->a_data == NULL) {
            return 1;
        }
        
        p_list->a_size = C_LIST_INITIAL_SIZE;
            
        return 0;
    }

    void *l_newData = krealloc(
        p_list->a_data, p_list->a_size
        * C_LIST_GROWTH_FACTOR
        * C_LIST_ELEMENT_SIZE
    );

    if(l_newData == NULL) {
        return 1;
    }

    p_list->a_size *= C_LIST_GROWTH_FACTOR;

    return 0;
}
