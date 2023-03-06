#include <string.h>

#include <kernel/klibc/stdlib.h>

char *kstrdup(const char *p_src) {
    size_t l_length = strlen(p_src) + 1;
    char *l_copy = kmalloc(l_length);

    if(l_copy == NULL) {
        return NULL;
    }

    memcpy(l_copy, p_src, l_length);

    return l_copy;
}
