#include "tty/tty.hpp"

namespace kernel {
    extern TTY kernel_tty;

    void debug(const char *message) {
        kernel_tty.puts(message);
    }

    extern "C" void kernel_debug(const char *message) {
        debug(message);
    }
}
