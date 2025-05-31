/*
Kernel types are defined as such:

char -> 8-bit signed integer
short -> 16-bit signed integer
int -> 16-bit signed integer
long -> 32-bit signed integer
pointer -> 16-bit segment followed by 16-bit offset
size_t -> 16-bit unsigned integer
*/

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int16_t;
typedef long int32_t;
typedef unsigned long uint32_t;
typedef uint16_t size_t;
typedef int16_t ssize_t;
typedef uint32_t lba_t;
typedef long off_t;
typedef uint16_t mode_t;
typedef uint16_t uid_t;
typedef uint16_t gid_t;
typedef uint32_t time_t;
