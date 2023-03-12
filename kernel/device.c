#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <kernel/device.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>

struct ts_vfsFileDescriptor *deviceCreate(const char *p_format, int p_first) {
    char l_buffer[PATH_MAX];
    int l_value = p_first;

    struct ts_vfsFileDescriptor *l_file;

    do {
        snprintf(l_buffer, sizeof(l_buffer), p_format, l_value);
        l_file = vfsFind(l_buffer);

        if(l_file != NULL) {
            l_value++;
        }
    } while(l_file != NULL);

    l_file = kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_file == NULL) {
        return NULL;
    }

    const char *l_lastSeparator = strrchr(l_buffer, '/');

    strcpy(l_file->a_name, l_lastSeparator + 1);

    return l_file;
}

int deviceMount(const char *p_path, struct ts_vfsFileDescriptor *p_file) {
    char l_buffer[PATH_MAX];

    snprintf(l_buffer, PATH_MAX, p_path, p_file->a_name);

    return vfsMount(l_buffer, p_file);
}
