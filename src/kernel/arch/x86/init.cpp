#include "kernel/arch/x86/mm/mm.hpp"

namespace kernel {
    namespace arch {
        void preinit() {
            x86::mm::mm::init();
        }

        void init() {

        }
    }
}
