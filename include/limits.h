#ifndef __INCLUDE_LIMITS_H__
#define __INCLUDE_LIMITS_H__

/* POSIX runtime invariant values (incomplete) */

/* POSIX pathname variable values (incomplete) */
#define FILESIZEBITS 64
#define NAME_MAX 255
#define PATH_MAX 1024

/* POSIX maximum values */
#define _POSIX_CLOCKRES_MIN 20000000

/* POSIX minimum values */
#define _POSIX_AIO_LISTIO_MAX 2
#define _POSIX_AIO_MAX 1
#define _POSIX_ARG_MAX 4096
#define _POSIX_CHILD_MAX 25
#define _POSIX_DELAYTIMER_MAX 32
#define _POSIX_HOST_NAME_MAX 255
#define _POSIX_LINK_MAX 8
#define _POSIX_LOGIN_NAME_MAX 9
#define _POSIX_MAX_CANON 255
#define _POSIX_MAX_INPUT 255
#define _POSIX_MQ_OPEN_MAX 8
#define _POSIX_MQ_PRIO_MAX 32
#define _POSIX_NAME_MAX 14
#define _POSIX_NGROUPS_MAX 8
#define _POSIX_OPEN_MAX 20
#define _POSIX_PATH_MAX 256
#define _POSIX_PIPE_BUF 512
#define _POSIX_RE_DUP_MAX 255
#define _POSIX_RTSIG_MAX 8
#define _POSIX_SEM_NSEMS_MAX 256
#define _POSIX_SEM_VALUE_MAX 32767
#define _POSIX_SIGQUEUE_MAX 32
#define _POSIX_SSIZE_MAX 32767
#define _POSIX_SS_REPL_MAX 4
#define _POSIX_STREAM_MAX 8
#define _POSIX_SYMLINK_MAX 255
#define _POSIX_SYMLOOP_MAX 8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS 4
#define _POSIX_THREAD_KEYS_MAX 128
#define _POSIX_THREAD_THREADS_MAX 64
#define _POSIX_TIMER_MAX 32
#define _POSIX_TRACE_EVENT_NAME_MAX 30
#define _POSIX_TRACE_NAME_MAX 8
#define _POSIX_TRACE_SYS_MAX 8
#define _POSIX_TRACE_USER_EVENT_MAX 32
#define _POSIX_TTY_NAME_MAX 9
#define _POSIX_TZNAME_MAX 6
#define _POSIX2_BC_BASE_MAX 99
#define _POSIX2_BC_DIM_MAX 2048
#define _POSIX2_BC_SCALE_MAX 99
#define _POSIX2_BC_STRING_MAX 1000
#define _POSIX2_CHARCLASS_NAME_MAX 14
#define _POSIX2_COLL_WEIGHTS_MAX 2
#define _POSIX2_EXPR_NEST_MAX 32
#define _POSIX2_LINE_MAX 2048
#define _POSIX2_RE_DUP_MAX 255
#define _XOPEN_IOV_MAX 16
#define _XOPEN_NAME_MAX 255
#define _XOPEN_PATH_MAX 1024

/* POSIX numerical limits */
#define CHAR_BIT 8
#define CHAR_MAX SCHAR_MAX
#define CHAR_MIN SCHAR_MIN
#define INT_MAX ((int)0x7fffffff)
#define INT_MIN ((int)0x80000000)
#define LLONG_MAX ((long long)0x7fffffffffffffff)
#define LLONG_MIN ((long long)0x8000000000000000)
#define LONG_BIT 64
#define LONG_MAX ((long)0xffffffffffffffff)
#define LONG_MIN ((long)0x8000000000000000)
#define MB_LEN_MAX 1
#define SCHAR_MAX ((char)0x7f)
#define SCHAR_MIN ((char)0x80)
#define SHRT_MAX ((short)0x7fff)
#define SHRT_MIN ((short)0x8000)
#define SSIZE_MAX LONG_MAX
#define UCHAR_MAX ((unsigned char)0xff)
#define UINT_MAX ((unsigned int)0xffffffff)
#define ULLONG_MAX ((unsigned long long)0xffffffffffffffff)
#define ULONG_MAX ((unsigned long)0xffffffffffffffff)
#define USHRT_MAX ((unsigned short)0xffff)
#define WORD_BIT 32

/* POSIX other invariant values */
#define NL_ARGMAX 32
#define NL_LANGMAX 14
#define NL_MSGMAX 32767
#define NL_SETMAX 255
#define NL_TEXTMAX _POSIX2_LINE_MAX
#define NZERO 20

#endif
