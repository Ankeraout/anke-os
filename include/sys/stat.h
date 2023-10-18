#ifndef __INCLUDE_SYS_STAT_H__
#define __INCLUDE_SYS_STAT_H__

#define S_IFMT 07000
#define S_IFREG 00000
#define S_IFDIR 01000
#define S_IFBLK 02000
#define S_IFCHR 03000
#define S_IFLNK 04000
#define S_IFIFO 05000
#define S_IFSOCK 06000

#define S_ISREG(m) ((m & S_IFMT) == S_IFREG)
#define S_ISDIR(m) ((m & S_IFMT) == S_IFDIR)
#define S_ISBLK(m) ((m & S_IFMT) == S_IFBLK)
#define S_ISCHR(m) ((m & S_IFMT) == S_IFCHR)
#define S_ISLNK(m) ((m & S_IFMT) == S_IFLNK)
#define S_ISFIFO(m) ((m & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) ((m & S_IFMT) == S_IFSOCK)

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)

#endif
