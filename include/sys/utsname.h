#ifndef __INCLUDE_SYS_UTSNAME_H__
#define __INCLUDE_SYS_UTSNAME_H__

struct utsname {
    char sysname[16];
    char nodename[16];
    char release[16];
    char version[16];
    char machine[64];
};

int uname(struct utsname *);

#endif
