#ifndef __INCLUDE_KERNEL_PRINTK_H__
#define __INCLUDE_KERNEL_PRINTK_H__

#include <stdarg.h>

#define KERN_EMERG "0"
#define KERN_ALERT "1"
#define KERN_CRIT "2"
#define KERN_ERR "3"
#define KERN_WARNING "4"
#define KERN_NOTICE "5"
#define KERN_INFO "6"
#define KERN_DEBUG "7"

#define pr_fmt(p_format) p_format

#define pr_emerg(p_format, ...) \
    printk(KERN_EMERG pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_alert(p_format, ...) \
    printk(KERN_ALERT pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_crit(p_format, ...) \
    printk(KERN_CRIT pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_err(p_format, ...) \
    printk(KERN_ERR pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_warning(p_format, ...) \
    printk(KERN_WARNING pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_notice(p_format, ...) \
    printk(KERN_NOTICE pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_info(p_format, ...) \
    printk(KERN_INFO pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)
#define pr_debug(p_format, ...) \
    printk(KERN_DEBUG pr_fmt(p_format) __VA_OPT__(,) __VA_ARGS__)

void printk(const char *p_format, ...);
void vprintk(const char *p_format, va_list p_args);

#endif
