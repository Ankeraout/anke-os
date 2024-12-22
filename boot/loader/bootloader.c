#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/arch/x86/idt.h"
#include "boot/loader/arch/x86/isr.h"
#include "boot/loader/arch/x86/pic.h"
#include "boot/loader/boot.h"
#include "boot/loader/drivers/block/floppy.h"
#include "boot/loader/drivers/cmos.h"
#include "boot/loader/drivers/console/console.h"
#include "boot/loader/drivers/console/fbcon.h"
#include "boot/loader/mm.h"
#include "boot/loader/stdio.h"
#include "boot/loader/stdlib.h"
#include "boot/loader/acpi/acpi.h"

static void test(void);
static void acpi_init(void);

int main(const struct ts_bootInfoStructure *p_bootInfoStructure) {
    mm_init(p_bootInfoStructure);

    framebuffer_init(p_bootInfoStructure);
    console_init();
    fbcon_init();

    printf("AnkeOS bootloader 0.1.0\n");

    // Initialize interrupts
    idt_init();
    isr_init();
    pic_init();
    sti();

    // Initialize ACPI
    acpi_init();

    test();

    printf("Initialization complete.\n");

    while(1) {
        asm("hlt");
    }
}

static void test() {
    floppy_init();
}

static void acpi_init(void) {
    printf("Initializing ACPI...\n");

    struct ts_acpi l_acpi;

    const struct ts_acpiRsdp *l_rsdp = acpiRsdpLocate(
        (const void *)0xe0000,
        0x20000
    );

    if(l_rsdp == NULL) {
        printf("ACPI RSDP not found.\n");
    } else {
        printf("ACPI RSDP found.\n");

        int l_result = acpiInit(&l_acpi, l_rsdp);

        if(l_result != 0) {
            printf("ACPI initialization failed.\n");
        } else {
            printf("ACPI initialized.\n");
            printf("  - RSDP at %p\n", l_acpi.m_rsdp);
            printf("  - RSDT at %p\n", l_acpi.m_rsdt);
            printf("  - FADT at %p\n", l_acpi.m_fadt);
            printf("  - MADT at %p\n", l_acpi.m_madt);
            printf("  - DSDT at %p\n", l_acpi.m_dsdt);
            printf("  - XSDT at %p\n", l_acpi.m_xsdt);
        }
    }
}
