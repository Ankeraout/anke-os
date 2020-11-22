#include "arch/i686/io.hpp"

namespace kernel {
    void halt() {
        while(true) {
            cli();
            hlt();
        }
    }
}
