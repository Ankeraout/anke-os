#include "tty/tty.hpp"

namespace kernel {
    extern TTY kernel_tty;
    extern void halt();

    void panic(const char *message) {
        kernel_tty.setAttr(0x0c);
        kernel_tty.puts("Kernel panic!\n");
        kernel_tty.puts(message);

        halt();
    }

    extern "C" void kernel_panic(const char *message) {
        panic(message);
    }
}
