#include <stdbool.h>
#include <stdint.h>

#include "debug.hpp"
#include "irq.hpp"
#include "arch/arch.hpp"
#include "libk/libk.hpp"
#include "tty/tty.hpp"

namespace kernel {
    void kmain() {
        arch_preinit();

        irq_init();

        debug("       db                      88                      ,ad8888ba,     ad88888ba ");
        debug("      d88b                     88                     d8\"'    `\"8b   d8\"     \"8b");
        debug("     d8'`8b                    88                    d8'        `8b  Y8,        ");
        debug("    d8'  `8b      8b,dPPYba,   88   ,d8   ,adPPYba,  88          88  `Y8aaaaa,  ");
        debug("   d8YaaaaY8b     88P'   `\"8a  88 ,a8\"   a8P_____88  88          88    `\"\"\"\"\"8b,");
        debug("  d8\"\"\"\"\"\"\"\"8b    88       88  8888[     8PP\"\"\"\"\"\"\"  Y8,        ,8P          `8b");
        debug(" d8'        `8b   88       88  88`\"Yba,  \"8b,   ,aa   Y8a.    .a8P   Y8a     a8P");
        debug("d8'          `8b  88       88  88   `Y8a  `\"Ybbd8\"'    `\"Y8888Y\"'     \"Y88888P\" ");
        debug("\n");
        debug("Welcome to AnkeOS!\n");

        arch_init();

        while(true) {
            halt();
        }
    }
}

extern "C" void kernel_main() {
    kernel::kmain();
}
