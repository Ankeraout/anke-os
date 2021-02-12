#include <stddef.h>

#include "libk/list.h"
#include "libk/stdlib.h"

list_t *list_create();
void list_add(list_t *list, void *element);
void list_remove(list_t *list, void *element);
void list_removeAt(list_t *list, size_t index);
void list_insert(list_t *list, size_t index, void *element);
void list_destroy(list_t *list);
static void list_expand(list_t *list);

list_t *list_create() {
    list_t *newList = malloc(sizeof(list_t));

    newList->elements = malloc(1 * sizeof(void *));
    newList->length = 0;
    newList->internalLength = 1;

    return newList;
}

void list_add(list_t *list, void *element) {
    if(list->internalLength == list->length) {
        list_expand(list);
    }

    list->elements[list->length - 1] = element;
    list->length++;
}

void list_remove(list_t *list, void *element) {
    for(size_t i = 0; i < list->length; i++) {
        if(list->elements[i] == element) {
            list_removeAt(list, i);
            break;
        }
    }
}

void list_removeAt(list_t *list, size_t index) {
    for(size_t i = index; i < list->length - 1; i++) {
        list->elements[i] = list->elements[i + 1];
    }

    list->length--;
}

void list_insert(list_t *list, size_t index, void *element) {
    if(index > list->internalLength) {
        return;
    }

    if(list->internalLength == list->length) {
        list_expand(list);
    }
}

void list_destroy(list_t *list) {
    free(list->elements);
    free(list);
}

static void list_expand(list_t *list) {
    void **elements = malloc(list->internalLength * 2 * sizeof(void *));

    for(int i = 0; i < list->internalLength; i++) {
        elements[i] = list->elements[i];
    }

    free(list->elements);

    list->elements = elements;
    list->internalLength *= 2;
}
