#include "arch/arch.h"
#include "tty.h"

void kernel_main() {
    arch_init();

    tty_write(&kernel_tty, "arch_init() done.\n");
    tty_write(&kernel_tty, "Welcome to AnkeOS!\n");

    arch_disableInterrupts();
    arch_halt();
}
