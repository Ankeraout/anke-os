#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/arch/x86/idt.h"
#include "boot/loader/arch/x86/isr.h"
#include "boot/loader/arch/x86/pic.h"
#include "boot/loader/boot.h"
#include "boot/loader/drivers/block/floppy.h"
#include "boot/loader/drivers/cmos.h"
#include "boot/loader/drivers/console/console.h"
#include "boot/loader/drivers/console/fbcon.h"
#include "boot/loader/stdio.h"

static void test(void);

int main(const struct ts_bootInfoStructure *p_bootInfoStructure) {
    framebuffer_init(p_bootInfoStructure);
    console_init();
    fbcon_init();

    printf("AnkeOS bootloader 0.1.0\n");

    // Initialize interrupts
    idt_init();
    isr_init();
    pic_init();
    sti();

    test();

    printf("Initialization complete.\n");

    while(1) {
        asm("hlt");
    }
}

static void test() {
    floppy_init();
}
