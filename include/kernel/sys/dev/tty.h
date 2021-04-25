#ifndef __KERNEL_SYS_DEV_TTY_H__
#define __KERNEL_SYS_DEV_TTY_H__

#include <stddef.h>

typedef struct tty_driver_s {
    void (*write)(struct tty_driver_s *driver, const void *str, size_t n);
} tty_driver_t;

void tty_driver_register(tty_driver_t *driver);
void tty_driver_input(tty_driver_t *driver, const void *str, size_t n);
const tty_driver_t *tty_getDefault();

#endif
