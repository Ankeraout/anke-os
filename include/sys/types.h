#ifndef __INCLUDE_SYS_TYPES_H__
#define __INCLUDE_SYS_TYPES_H__

#include <stdint.h>

typedef long ssize_t;
typedef long loff_t;
typedef int mode_t;
typedef int gid_t;
typedef int uid_t;

typedef struct {
    uint8_t major;
    uint8_t minor;
} dev_t;

#endif
