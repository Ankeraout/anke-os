#include "printk.h"

void panic(const char *p_message) {
    pr_crit("===== KERNEL PANIC =====\n");
    pr_crit("Message: %s\n", p_message);
    pr_crit("========================\n");
    pr_crit("System halted.\n");

    while(1);
}
