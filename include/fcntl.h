#ifndef __INCLUDE_FCNTL_H__
#define __INCLUDE_FCNTL_H__

enum {
    F_DUPFD = 1,
    F_DUPFD_CLOEXEC,
    F_GETFD,
    F_SETFD,
    F_GETFL,
    F_SETFL,
    F_GETLK,
    F_SETLK,
    F_SETLKW,
    F_GETOWN,
    F_SETOWN
};

#define FD_CLOEXEC (1 << 0)

enum {
    F_RDLCK = 1,
    F_UNLCK,
    F_WRLCK
};

#define O_CLOEXEC (1 << 0)
#define O_CREAT (1 << 1)
#define O_DIRECTORY (1 << 2)
#define O_EXCL (1 << 3)
#define O_NOCTTY (1 << 4)
#define O_NOFOLLOW (1 << 5)
#define O_TRUNC (1 << 6)
#define O_TTY_INIT (1 << 7)

#define O_APPEND (1 << 8)
#define O_DSYNC (1 << 9)
#define O_NONBLOCK (1 << 10)
#define O_RSYNC (1 << 11)
#define O_SYNC (1 << 12)

#define O_EXEC (1 << 13)
#define O_RDONLY (1 << 14)
#define O_RDWR (1 << 15)
#define O_SEARCH (1 << 16)
#define O_WRONLY (1 << 17)

#endif
