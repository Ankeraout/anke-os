#ifndef __INCLUDE_DIRENT_H__
#define __INCLUDE_DIRENT_H__

#include <limits.h>
#include <sys/types.h>

struct dirent {
    ino_t d_ino;
    char d_name[NAME_MAX + 1];
};

#endif
