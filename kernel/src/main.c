#include "acpi/acpi.h"
#include "arch/arch.h"
#include "arch/amd64/gdt.h"
#include "bootstrap.h"
#include "printk.h"

void main(void) {
    bootstrap();
    arch_init();

    // Initialize ACPI
    const struct ts_acpiRsdp *l_acpiRsdp =
        acpiRsdpLocate((const void *)0xe0000, 0x20000);

    acpiSetRsdpLocation(l_acpiRsdp);

    int result = acpiInit();

    if(result == 0) {
        pr_info("ACPI initialized.\n");
    } else {
        pr_info("ACPI initialization failed: %d\n", result);
    }

    printk("Hello, World!\n");

    while(1) {
        asm("hlt");
    }
}
